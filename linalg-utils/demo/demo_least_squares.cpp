
#include <ctime>
#include <linalg/linalg.hpp>
#include <opencv2/opencv.hpp>
#include <random>
#include <vector>

using namespace linalg;
using namespace linalg::operators;

enum
{
    POINTS,
    LINES,
};

template <typename T>
void plot(std::string name, std::vector<T> &data, matrix &beta, matrix &domain) {
    cv::Scalar blue(255, 0, 0);
    cv::Scalar red(0, 0, 255);
    cv::Scalar black(0, 0, 0);
    int border = 20;
    int b2 = border >> 1;
    T x_min, y_min = x_min = std::numeric_limits<T>::max();
    T x_max, y_max = x_max = std::numeric_limits<T>::min();
    for (auto &d : data) {
        if (y_min > d) y_min = d;
        if (y_max < d) y_max = d;
    }
    T y_range = y_max - y_min;
    printf("y_min:%lf y_max:%lf y_range:%lf\n", y_min, y_max, y_range);
    domain.for_each ([&](double &v) {
        if (x_min > v) x_min = v;
        if (x_max < v) x_max = v;
    });
    T x_range = x_max - x_min;
    printf("x_min:%lf x_max:%lf x_range:%lf\n", x_min, x_max, x_range);

    size_t width = static_cast<size_t>(x_range);
    size_t height = static_cast<size_t>(y_range);
    printf("Active area: %zux%zu\n", width, height);

    cv::Size window_size(border + width, border + height);
    printf("Window: %ux%u\n", window_size.width, window_size.height);
    cv::Mat img(window_size, CV_8UC3);

    img = cv::Scalar::all(255);  // clear the image with white
    // where will the render offset be?
    int x_off = b2;
    int y_off = b2;
    int x0 = x_off;
    int y0 = y_off;

    // find x zero
    if (x_min > 0 || x_max < 0) {
        x0 = x_off + (x_range) / 2;
    } else {
        x0 = x_off + (0 - x_min);
    }
    // find y zero
    if (y_min > 0 || y_max < 0) {
        // this won't cross the zero line
        y0 = y_off + (y_range) / 2;
    } else {
        y0 = y_off + (0 - y_min);
    }

    cv::Point y_top(x0, y0 - (y_range / 2));
    cv::Point y_btm(x0, y0 + (y_range / 2));
    cv::line(img, y_top, y_btm, blue, 2);

    cv::Point x_left(x0 - (x_range / 2), y0);
    cv::Point x_right(x0 + (x_range / 2), y0);
    cv::line(img, x_left, x_right, red, 2);

    for (size_t i = 0; i < domain.cols; i++) {
        int x = x0 + static_cast<int>(domain[0][i]);
        int y = y0 - data[i];
        cv::Point pt{x, y};
        cv::circle(img, pt, 1, black, 1);
    }

    // find min,max domain intersections of the beta line
    T y1 = beta.at(2, 1) * x_min + beta.at(1, 1);
    T y2 = beta.at(2, 1) * x_max + beta.at(1, 1);
    printf("beta y1:%lf, y2:%lf\n", y1, y2);
    cv::Point beta_left(x0 + x_min, y0 - y1);
    cv::Point beta_right(x0 + x_max, y0 - y2);
    printf("x1,y1=%d,%d  x2,y2=%d,%d\n", beta_left.x, beta_left.y, beta_right.x, beta_right.y);
    cv::line(img, beta_left, beta_right, black, 1);

    do {
        cv::imshow(name, img);
        int key = cv::waitKey(100);
        switch (key & 0xFFFF) {
            case 'q':
                return;
            default:
                break;
        }
    } while (1);
}

double frand() {
    return static_cast<double>(rand()) / RAND_MAX;
}

double random_range(double min, double max) {
    double range = max - min;
    return (frand() * range) + min;
}

std::vector<double> generate_random_linear_dataset(matrix &domain) {
    std::random_device rd{};
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> num(-1.0, 1.0);
    // slope
    double m = 2.0 * num(gen);
    // offset
    double b = 10.0 * num(gen);
    // we want the dots clustered around a random line equation by this tolerance.
    double tolerance = 20.0 * num(gen);
    printf("m=%lf, b=%lf\n", m, b);
    std::vector<double> data;
    domain.for_each ([&](double &x) {
        double y = m * x + b;
        std::uniform_real_distribution<> rnd(y - tolerance, y + tolerance);
        // printf("%lf = m * %lf + b\n", y, x);
        y = rnd(gen);
        data.push_back(y);
    });
    return data;
}

void least_squares(std::vector<double> &dataset, matrix &beta, matrix &domain) {
    // create a column matrix of the data points (default is row)
    matrix y = matrix::col(dataset);
    matrix X = matrix::ones(dataset.size(), 2);
    // copy the domain into the X matrix
    domain.T().assignInto(X, 0, 1);
    beta = ((X ^ T) * X).inverse() * (X ^ T) * y;
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    matrix domain(1, 300);  // generate a domain set
    domain.for_each ([](size_t row, size_t col, double &v) {
        row |= 0;
        v = -150.0 + col;
    });
    // generate a random data set within bounds
    std::vector<double> dataset = generate_random_linear_dataset(domain);
    // std::vector<double> smallset{{6,5,7,10}};
    matrix beta(2, 1);
    least_squares(dataset, beta, domain);
    beta.print("beta");
    plot("data", dataset, beta, domain);

    return 0;
}
