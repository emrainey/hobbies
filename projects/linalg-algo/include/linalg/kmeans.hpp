#pragma once
/// @file
/// Definitions for the linalg::kmeans object.
/// @copyright Copyright 2019 (C) Erik Rainey.

#include <geometry/geometry.hpp>
#include <linalg/linalg.hpp>
#include <vector>

/// The K-Means algorithm namespace
namespace linalg {

/// The K-Means Object
class kmeans {
public:
    /// An enumeration to describe how to initialize the initial points
    enum InitialMethod : int {
        RandomPoints,  ///< Use a random assignment of values.
    };

    /// An enumeration to describe how to iterate over the points
    enum IterationMethod : int {
        EuclideanDistance,  ///< Use the euclidean distance
    };

    /// Constructs from a set of 2 points
    kmeans(std::vector<geometry::R2::point> &points);

    /// Destructor
    ~kmeans();

    /// Tells the algorithm how many cluster points to use and which method to estimate the first steps
    /// @param N A number > 1 and < points.length / 4
    void configure(size_t N);

    /// Creates the first cluster points via some equations
    void initial(InitialMethod method);

    /// Iterates the method and reports the error
    precision iteration(IterationMethod method);

    /// Returns the vector of centroids.
    const std::vector<geometry::R2::point> &centroids() const;

    /// Returns the list of indexes (associates which point is in which cluster)
    const std::vector<size_t> &indexes() const;

private:
    /// The vector of external points, P long
    std::vector<geometry::R2::point> &m_points;

    /// The internal vector of centroids, N long
    std::vector<geometry::R2::point> m_centroids;

    /// The internal vector of cluster assignment, P long
    std::vector<size_t> m_cluster_assignment;
};

}  // namespace linalg