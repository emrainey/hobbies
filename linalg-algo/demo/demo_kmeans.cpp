#include <linalg/linalg.hpp>
#include <linalg/utils.hpp>
#include <vector>

#include "linalg/kmeans.hpp"

using namespace linalg;
using namespace geometry;

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    // std::default_random_engine re;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> unif(-120, 120);
    std::vector<std::normal_distribution<double>> distribs(8);
    for (auto &dist : distribs) {
        dist = std::normal_distribution<double>(unif(gen), rand() % 200);
    }
    // create a bunch of random points
    std::vector<point_<2>> points(6000);
    for (size_t i = 0; i < points.size(); i++) {
        int r = rand() % 4;
        points[i].x = distribs[r](gen);
        points[i].y = distribs[r + 1](gen);
    }
    kmeans km(points);
    km.configure(4);
    std::vector<cv::Scalar> colors{{
        cv::Scalar(0, 255, 0, 0),   // Green
        cv::Scalar(255, 0, 0, 0),   // Red
        cv::Scalar(0, 0, 255, 0),   // Blue
        cv::Scalar(255, 0, 255, 0)  // Magenta
    }};
    double error = std::numeric_limits<double>::max();
    size_t iteration = 0;
    km.initial(kmeans::InitialMethod::RandomPoints);
    do {
        error = km.iteration(kmeans::IterationMethod::EuclideanDistance);
        iteration++;
        printf("Iteration %zu, Error %lf\r", iteration, error);
        linalg::plot_points(std::string("k-means"), points, km.indexes(), colors, 1);
    } while (error > 0.1);
    printf("Error is %lf after %zu iterations\n", error, iteration);
    linalg::plot_points(std::string("k-means"), points, km.indexes(), colors, 120);
    return 0;
}
