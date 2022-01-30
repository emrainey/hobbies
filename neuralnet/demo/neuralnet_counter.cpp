
#include <linalg/opencv.hpp>
#include "neuralnet/neuralnet.hpp"

using namespace linalg::operators;

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    try {

        // this does the heavy lifting...
        nn::network net{ {3,4,3} };
        net.set(nn::activation_type::Sigmoid);
        // these are convenience handles
        nn::input&  in  = net.as_input(0);
        nn::output& out = net.as_output(2);

        printf("Sigmoid of 300.0 => %lf\n", nn::sigmoid(300.0));
        assert(1.0 <= nn::sigmoid(300.0));
        printf("Sigmoid of -300.0 => %lf\n", nn::sigmoid(-300.0));
        assert(basal::equals(0.0,nn::sigmoid(-300.0)));

        linalg::matrix A{ {{5},
                        {6}}};
        linalg::matrix B = linalg::hadamard(A, A);
        double tot = nn::sum(B);
        double gnd = 25.0 + 36.0;
        (void)tot; (void)gnd;
        assert(tot <= gnd);

        // this data set is about producing the next value
        linalg::matrix dataset{ {
            {0.0, 0.0, 0.0},
            {0.0, 0.0, 1.0},
            {0.0, 1.0, 0.0},
            {0.0, 1.0, 1.0},
            {1.0, 0.0, 0.0},
            {1.0, 0.0, 1.0},
            {1.0, 1.0, 0.0},
            {1.0, 1.0, 1.0},
        } };
        linalg::matrix datasetT = dataset.transpose();
        linalg::matrix labelset{ {
            {0.0, 0.0, 1.0},
            {0.0, 1.0, 0.0},
            {0.0, 1.0, 1.0},
            {1.0, 0.0, 0.0},
            {1.0, 0.0, 1.0},
            {1.0, 1.0, 0.0},
            {1.0, 1.0, 1.0},
            {0.0, 0.0, 0.0},
        } };
        linalg::matrix labelsetT = labelset.transpose();
        // train
        const size_t reps = 60000;
        double alpha = -0.25;
        double gamma = -0.9;
        double last_rms = std::numeric_limits<double>::infinity();
        for (size_t r = 0; r < reps; r++) {
            net.reset();
            for (size_t idx = 0; idx < labelsetT.cols; idx++) {
            //for (size_t idx = 7; idx < 8; idx++) {
                // set the values from the row of the data set
                in.values = datasetT.col(idx);
                net.forward();
                out.learn_label(labelsetT.col(idx));
                net.backward(alpha, gamma);
                //net.visualize(std::chrono::milliseconds(1));
            }
            // now apply the learning
            net.update();
            //net.visualize();
            // make sure the number makes sense
            assert(not std::isnan(out.rms_value));
            if (out.rms_value > last_rms) {
                //printf("RMS is not improving!!!\n");
                //alpha += 0.001;
            } else if (basal::equals(out.rms_value, last_rms)) {
                //alpha -= 0.001;
            }
            // save the old number
            last_rms = out.rms_value;
            if ((r % (reps/10)) == 0) {
                printf("Iteration %zu RMS=%lf\n", r, out.rms_value);
            }
            if (out.rms_value < 0.0005) {
                printf("Took %zu iterations to get to success rate of %lf\n", r, out.rms_value);
                break;
            }
        }
        for (size_t idx = 0; idx < labelsetT.cols; idx++) {
        //for (size_t idx = 7; idx < 8; idx++) {
            // set the values from the row of the data set
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
