#include <linalg/linalg.hpp>
#include <opencv2/opencv.hpp>

#include "linalg/utils.hpp"

int main(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {
    {
        linalg::matrix in{1024, 1024};
        in.for_each ([&](int y, int x, double& v) {
            double lr = 2.0 / in.rows;
            v = ((double)y * lr + ((double)x / in.cols) * lr) - 1.0;
        });
        printf("mat[%d][%d] = %lf\n", 0, 0, in[0][0]);
        printf("mat[%d][%d] = %lf\n", 0, 1023, in[0][1023]);
        printf("mat[%d][%d] = %lf\n", 1023, 0, in[1023][0]);
        printf("mat[%d][%d] = %lf\n", 1023, 1023, in[1023][1023]);
        cv::Mat img = linalg::convert(in, CV_8UC3);
        cv::imshow("Jet Image", img);
    }
    {
        linalg::matrix in{784, 1};
        in.for_each ([&](int y, int x, double& v) {
            double lr = 2.0 / in.rows;
            v = ((double)y * lr + ((double)x / in.cols) * lr) - 1.0;
        });
        cv::Mat img = linalg::squarish(in, CV_8UC3);
        assert(img.rows == img.cols);
        cv::imshow("Squared", img);
    }
    {
        linalg::matrix in{4, 784};
        in.for_each ([&](int y, int x, double& v) {
            double lr = 2.0 / in.rows;
            v = ((double)y * lr + ((double)x / in.cols) * lr) - 1.0;
        });
        cv::Mat img = linalg::each_row_square_tiled(in, CV_8UC3, 2);
        assert(img.rows == img.cols);
        cv::imshow("Row Tiles", img);
    }
    {
        linalg::matrix in{10, 16};  // this makes a 3x4 with 2 empty cubes
        in.for_each ([&](int y, int x, double& v) {
            double lr = 2.0 / in.rows;
            v = ((double)y * lr + ((double)x / in.cols) * lr) - 1.0;
        });
        cv::Mat img = linalg::each_row_square_tiled(in, CV_8UC3, 4);
        // test_statement(img.rows == img.cols);
        cv::imshow("Not-square", img);
    }
    cv::waitKey(20000);
    return 0;
}
