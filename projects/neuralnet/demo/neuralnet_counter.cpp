
#include <linalg/opencv.hpp>

#include "neuralnet/neuralnet.hpp"

using namespace linalg::operators;
using namespace basal::literals;

int main(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {
    try {
        // this does the heavy lifting...
        nn::network net{{3, 4, 3}};
        net.set(nn::activation_type::Sigmoid);
        // these are convenience handles
        nn::input& in = net.as_input(0);
        nn::output& out = net.as_output(2);

        printf("Sigmoid of 300.0_p => %lf\n", nn::sigmoid(300.0_p));
        assert(1.0_p <= nn::sigmoid(300.0_p));
        printf("Sigmoid of -300.0_p => %lf\n", nn::sigmoid(-300.0_p));
        assert(basal::nearly_equals(0.0_p, nn::sigmoid(-300.0_p)));

        linalg::matrix A{{{5}, {6}}};
        linalg::matrix B = linalg::hadamard(A, A);
        nn::precision tot = nn::sum(B);
        nn::precision gnd = 25.0_p + 36.0_p;
        (void)tot;
        (void)gnd;
        assert(tot <= gnd);

        // this data set is about producing the next value
        linalg::matrix dataset{{
            {0.0_p, 0.0_p, 0.0_p},
            {0.0_p, 0.0_p, 1.0_p},
            {0.0_p, 1.0_p, 0.0_p},
            {0.0_p, 1.0_p, 1.0_p},
            {1.0_p, 0.0_p, 0.0_p},
            {1.0_p, 0.0_p, 1.0_p},
            {1.0_p, 1.0_p, 0.0_p},
            {1.0_p, 1.0_p, 1.0_p},
        }};
        linalg::matrix datasetT = dataset.transpose();
        linalg::matrix labelset{{
            {0.0_p, 0.0_p, 1.0_p},
            {0.0_p, 1.0_p, 0.0_p},
            {0.0_p, 1.0_p, 1.0_p},
            {1.0_p, 0.0_p, 0.0_p},
            {1.0_p, 0.0_p, 1.0_p},
            {1.0_p, 1.0_p, 0.0_p},
            {1.0_p, 1.0_p, 1.0_p},
            {0.0_p, 0.0_p, 0.0_p},
        }};
        linalg::matrix labelsetT = labelset.transpose();
        // train
        const size_t reps = 60000;
        nn::precision alpha = -0.25_p;
        nn::precision gamma = -0.9_p;
        nn::precision last_rms = std::numeric_limits<nn::precision>::infinity();
        for (size_t r = 0; r < reps; r++) {
            net.reset();
            for (size_t idx = 0; idx < labelsetT.cols; idx++) {
                // for (size_t idx = 7; idx < 8; idx++) {
                //  set the values from the row of the data set
                in.values = datasetT.col(idx);
                net.forward();
                out.learn_label(labelsetT.col(idx));
                net.backward(alpha, gamma);
                // net.visualize(std::chrono::milliseconds(1));
            }
            // now apply the learning
            net.update();
            // net.visualize();
            //  make sure the number makes sense
            assert(not basal::is_nan(out.rms_value));
            if (out.rms_value > last_rms) {
                // printf("RMS is not improving!!!\n");
                // alpha += 0.001_p;
            } else if (basal::nearly_equals(out.rms_value, last_rms)) {
                // alpha -= 0.001_p;
            }
            // save the old number
            last_rms = out.rms_value;
            if ((r % (reps / 10)) == 0) {
                printf("Iteration %zu RMS=%lf\n", r, out.rms_value);
            }
            if (out.rms_value < 0.0005_p) {
                printf("Took %zu iterations to get to success rate of %lf\n", r, out.rms_value);
                break;
            }
        }
        for (size_t idx = 0; idx < labelsetT.cols; idx++) {
            // for (size_t idx = 7; idx < 8; idx++) {
            //  set the values from the row of the data set
            in.values = datasetT.col(idx);
            net.forward();
            out.learn_label(labelsetT.col(idx));
            linalg::matrix a = (in.values | out.values);
            linalg::matrix b = (a | out.beta);
            linalg::matrix c = (b | out.rms);
            c.print("in/out/beta/rms");
            printf("Sum of Errors: %lf\n", out.rms_value);
            net.visualize(std::chrono::milliseconds(100000));
        }
        return 0;
    } catch (basal::exception& e) {
        printf("Exception in test! %s %s\n", e.what(), e.why());
        return -1;
    }
}
