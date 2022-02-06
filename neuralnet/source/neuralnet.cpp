
#include "neuralnet/neuralnet.hpp"
#include <unistd.h>

using namespace linalg;

/** The learning rate of the network */
namespace local {
constexpr double alpha = -0.25;
constexpr double gamma = -0.9;
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    try {
        constexpr const uint32_t num = nn::mnist::inputs;
        char path[256] = {0};
        getcwd(path, sizeof(path));
        printf("Currently running in %s\n", path);

        // Create the network
        std::vector<size_t> node_counts{{num, 16, 16, 10}};

        // call the list constructor
        nn::network net(node_counts);
        net.set(nn::activation_type::Tanh);

        // Create some handles
        nn::input&  in = net.as_input(0);
        nn::output& out = net.as_output(3);

        // these are the visualization images of the hidden layers
        const char *named_i1 = "input";

        nn::mnist learning("train-labels-idx1-ubyte", "train-images-idx3-ubyte", 6000);
        if (not learning.load()) {
            return -1;
        }
        nn::mnist testdata("t10k-labels-idx1-ubyte", "t10k-images-idx3-ubyte", 1000);
        if (not testdata.load()) {
            return -1;
        }
        constexpr static const uint32_t reps = 600;
        const int minibatch = 2000;

        for (uint32_t r = 0; r < reps; r++) {
            printf("Starting rep %u...\n", r); fflush(stdout);
            net.reset();
            for (uint32_t idx = 0; idx < learning.get_num_entries(); idx++) {
                uint8_t answer = learning.get_label(idx);
                in.encode(learning, idx);
                net.forward();
                out.learn_label(answer, 0.0, 1.0);
                net.backward(local::alpha, local::gamma);
                if (idx > 0 and (idx % minibatch) == 0) {
                    net.visualize(std::chrono::milliseconds(1));
                    net.update();
                    printf("RMS: %0.5lf\n", out.rms_value);
                    net.reset();
                }
            }
            net.update();
            printf("RMS: %0.5lf\n", out.rms_value);

            if (out.rms_value < 0.005) {
                printf("Done!\n");
                break;
            }
        }
        uint32_t misses = 0, hits = 0;

        for (uint32_t i = 0; i < testdata.get_num_entries(); i++) {
            printf("Starting data item %u...", i); fflush(stdout);
            in.encode(testdata, i);
            cv::Mat inm = testdata.get_mat(i);
            cv::imshow(named_i1, inm);
            net.forward();

            double value = 0.0;
            size_t idx = out.infer_label(value);
            if (idx == testdata.get_label(i)) {
                hits++;
            } else {
                misses++;
            }
            printf("Complete! %zu => %lf (hits=%u, misses=%u total=%u rate:%lf)\n",
                    idx, value, hits, misses, i, float(hits+misses)/i);
            fflush(stdout);

            out.values.T().print("output");
        }
        return 0;
    } catch (basal::exception& e) {
        printf("Failed somewhere in %s! %s\n", __FUNCTION__, e.why());
        return -1;
    }
}
