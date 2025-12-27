
#include "neuralnet/neuralnet.hpp"

#include <unistd.h>

#include <basal/options.hpp>

using namespace linalg;

/// The learning rate of the network - now configurable via command line

int main(int argc, char* argv[]) {
    std::string training_labels("train-labels-idx1-ubyte");
    std::string training_images("train-images-idx3-ubyte");
    std::string data_labels("t10k-labels-idx1-ubyte");
    std::string data_images("t10k-images-idx3-ubyte");

    basal::options::config opts[] = {{"-tl", "--train-labels", training_labels, "Training Data Labels"},
                                     {"-ti", "--train-images", training_images, "Training Data Images"},
                                     {"-dl", "--data-labels", data_labels, "Data Labels"},
                                     {"-di", "--data-images", data_images, "Data Images"},
                                     {"-c", "--count", int(10), "Number of epochs (repetitions through dataset)"},
                                     {"-m", "--minibatch", int(50u), "Number of items in a minibatch"},
                                     {"-a", "--alpha", precision(0.00390625_p), "Learning rate (power of 2: 1/256)"},
                                     {"-g", "--gamma", precision(0.0625_p), "Momentum factor (power of 2: 1/16)"},
                                     {"-r", "--reset", false, "Forces a reset between each minibatch"},
                                     {"-v", "--visualize", false, "Enables visualization of the network during training"},
                                     {"-t", "--test-limit", int(1000), "Limit number of test samples for quick evaluation"}};
    try {
        constexpr uint32_t const num = nn::mnist::inputs;
        char path[256] = {0};
        (void)getcwd(path, sizeof(path));
        printf("Currently running in %s\n", path);

        // Create a simpler network to start - avoid overfitting
        std::vector<size_t> node_counts{{num, 32, 10}};

        // call the list constructor
        nn::network net(node_counts);
        net.set(nn::activation_type::Sigmoid);  // Keep sigmoid for now

        // Create some handles
        nn::input& in = net.as_input(0);
        nn::output& out = net.as_output(2);  // Fixed: 3-layer network has output at index 2

        // these are the visualization images of the hidden layers
        char const* named_i1 = "input";

        int count;
        int minibatch;
        int test_limit;
        precision alpha;
        precision gamma;
        bool reset;
        bool visualize;

        basal::options::process(basal::dimof(opts), opts, argc, argv);
        basal::options::find(opts, "--train-labels", training_labels);
        basal::options::find(opts, "--train-images", training_images);
        basal::options::find(opts, "--data-labels", data_labels);
        basal::options::find(opts, "--data-images", data_images);
        basal::options::find(opts, "--count", count);
        basal::options::find(opts, "--minibatch", minibatch);
        basal::options::find(opts, "--test-limit", test_limit);
        basal::options::find(opts, "--alpha", alpha);
        basal::options::find(opts, "--gamma", gamma);
        basal::options::find(opts, "--reset", reset);
        basal::options::find(opts, "--visualize", visualize);
        basal::options::print(basal::dimof(opts), opts);

        nn::mnist learning(training_labels, training_images, 60000);
        if (not learning.load()) {
            return -1;
        }
        nn::mnist testdata(data_labels, data_images, 10000);
        if (not testdata.load()) {
            return -1;
        }

        for (uint32_t r = 0; r < count; r++) {
            printf("Starting rep %u...\n", r);
            fflush(stdout);
            net.reset();
            for (uint32_t idx = 0; idx < learning.get_num_entries(); idx++) {
                uint8_t answer = learning.get_label(idx);
                in.encode(learning, idx);
                net.forward();
                out.learn_label(answer, 0.8_p, 0.2_p);  // Moderate target strength with some smoothing
                net.backward(alpha, gamma);
                if (idx > 0 and (idx % minibatch) == 0) {
                    net.update();
                    if (visualize) {
                        printf("RMS: %0.5lf\n", out.rms_value);
                        net.visualize(std::chrono::milliseconds(1));
                    }
                    if (reset) {
                        net.reset();
                    }
                }
            }
            net.update();
            printf("RMS: %0.5lf\n", out.rms_value);

            if (out.rms_value < 0.005_p) {
                printf("Done!\n");
                break;
            }
        }
        uint32_t misses = 0, hits = 0;
        uint32_t test_count = std::min(test_limit, (int)testdata.get_num_entries());
        printf("Testing on %u samples (limit: %d)\n", test_count, test_limit);

        for (uint32_t i = 0; i < test_count; i++) {
            printf("Starting data item %u...", i);
            fflush(stdout);
            in.encode(testdata, i);
            if (visualize) {
                cv::Mat inm = testdata.get_mat(i);
                cv::imshow(named_i1, inm);
            }
            net.forward();

            precision value = 0.0_p;
            size_t idx = out.infer_label(value);
            uint8_t expected = testdata.get_label(i);
            if (idx == expected) {
                hits++;
            } else {
                misses++;
            }
            printf("Complete! predicted=%zu (%.3f) expected=%u (hits=%u, misses=%u total=%u accuracy:%.2f%%)\n",
                   idx, value, expected, hits, misses, hits + misses,
                   100.0f * float(hits) / float(hits + misses));
            fflush(stdout);

            // Only show detailed output for misclassifications
            if (idx != expected) {
                out.values.T().print(std::cout, "output (MISS)");
            }
            if (visualize) {
                std::chrono::milliseconds delay(1);
                int ms = static_cast<int>(delay.count());
                int key = cv::waitKey(ms);
                switch ((key & 0xFF)) {
                    case 'q':
                        exit(0);
                        break;
                    case ' ':
                        break;
                }
            }
        }
        return 0;
    } catch (basal::exception& e) {
        printf("Failed somewhere in %s! %s\n", __FUNCTION__, e.why());
        return -1;
    }
}
