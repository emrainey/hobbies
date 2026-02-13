#include "linalg/kmeans.hpp"

#include <geometry/geometry.hpp>

using namespace linalg;
using namespace geometry;

namespace linalg {
kmeans::kmeans(std::vector<geometry::R2::point>& points) : m_points{points}, m_centroids{}, m_cluster_assignment{} {
}

kmeans::~kmeans() {
}

void kmeans::configure(size_t N) {
    // remove all previous entries
    if (m_centroids.size() > 0)
        m_centroids.clear();
    // clear all the cluster assignments
    if (m_cluster_assignment.size() > 0)
        m_cluster_assignment.clear();
    // add new items
    for (size_t i = 0; i < N; i++) {
        m_centroids.push_back(R2::point());
    }
    for (size_t p = 0; p < m_points.size(); p++) {
        m_cluster_assignment.push_back(N);
    }
}

/// Creates the first cluster points via some equations
void kmeans::initial(InitialMethod method) {
    if (method == InitialMethod::RandomPoints) {
        printf("Initializing %zu centroids\n", m_centroids.size());
        for (size_t c = 0; c < m_centroids.size(); c++) {
            size_t r = rand() % m_points.size();
            m_centroids[c] = m_points[r];
            m_centroids[c].print(std::cout, "Initial Centroid:");
        }
    }
}

/// Iterates the method and reports the error
precision kmeans::iteration(IterationMethod method) {
    precision error = 0.0_p;

    auto euclidean = [](R2::point const& A, R2::point const& B) -> precision { return (A - B).quadrance(); };

    for (auto& cnt : m_centroids) {
        cnt.print(std::cout, "Centroid:");
    }

    // create a vector of metrics per centroid
    std::vector<precision> metrics(m_centroids.size());
    // for each point
    for (size_t p = 0; p < m_points.size(); p++) {
        precision min_metric = std::numeric_limits<precision>::max();
        size_t min_index = m_centroids.size();
        // compute distance to each centroids
        for (size_t m = 0; m < metrics.size(); m++) {
            if (method == IterationMethod::EuclideanDistance) {
                metrics[m] = euclidean(m_centroids[m], m_points[p]);
            }
            if (metrics[m] <= min_metric) {
                min_index = m;
                min_metric = metrics[m];
            }
        }
#if 0
        printf("Metrics: ");
        for (size_t m = 0; m < metrics.size(); m++) {
            printf("%E,%E=>%E ", m_centroids[m].x(), m_centroids[m].y(), metrics[m]);
        }
        printf("\n");
#endif
        // assign to cluster of shortest distance metric
        m_cluster_assignment[p] = min_index;
    }

    // now compute the new centroid of each cluster
    std::vector<R2::point> sums(m_centroids.size());
    for (auto& s : sums) {
        s.zero();
    }
    std::vector<size_t> counts(m_centroids.size());
    for (auto& c : counts) {
        c = 0ul;
    }
    for (size_t p = 0; p < m_points.size(); p++) {
        // move the sums points by the vector
        size_t cluster = m_cluster_assignment[p];
        if (cluster < m_centroids.size()) {
            sums[cluster] += vector_<2>{{m_points[p].x(), m_points[p].y()}};
            counts[cluster]++;
        }
    }
    // scale points (average)
    for (size_t c = 0; c < m_centroids.size(); c++) {
        printf("Counts[%zu] = %zu\n", c, counts[c]);
        if (counts[c] > 0) {
            R2::point center(sums[c].x() / counts[c], sums[c].y() / counts[c]);
            precision err = sqrt(euclidean(m_centroids[c], center));
            center.print(std::cout, "Computed Center:");
            printf("Center[%zu] = %lf, %lf (error = %lf)\n", c, center.x(), center.y(), err);
            error += err;
            m_centroids[c] = center;
        }
    }
    // now update the new centroids
    return error;
}

/// Returns the vector of centroids.
std::vector<R2::point> const& kmeans::centroids() const {
    return m_centroids;
}

/// Returns the vector of indexes.
std::vector<size_t> const& kmeans::indexes() const {
    return m_cluster_assignment;
}

}  // namespace linalg
