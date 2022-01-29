#pragma once
/**
 * @file
 * Definitions of the Discrete Kalman Filter and associated objects.
 * @copyright Copyright 2019 (C) Erik Rainey.
 */

#include <linalg/linalg.hpp>

namespace linalg {
    /** Discrete Kalman Filter */
    class dkf {
    protected:
        /** Number of States */
        size_t ns;
        /** Number of Inputs */
        size_t ni;
        /** Number of Measurements */
        size_t nm;
        /** The enum of time increments */
        enum time_inc : short {
            k_1 = 0, /**< The last time measurement */
            k, /**< The current time measurement */
            N /**< The number of time measurements */
        };
        /** The states of the filter */
        matrix x[N];
        /** \ref A The state transition model */
        /** \ref B The inputs model */
        /** \ref H The measurement model */
        matrix A, B, H, K, Q, R, I;
        /** The error covariance models */
        matrix P[N];
    public:
        /** No empty constructor */
        dkf() = delete;
        /** Normal constructor which declares the size of each model */
        dkf(size_t num_variables_in_state, size_t num_variables_in_input, size_t num_variables_in_measurement);
        // copy constructor
        // move constructor
        // copy assignment
        // move assignment
        ~dkf();

        /** The enumeration of model names */
        enum models : int {
            STATE_TRANSITION,   ///< The model which transforms the previous state to the next state
            INPUT_TRANSLATION,  ///< The model which transforms the input data into the state adjustment
            STATE_MEASUREMENT,  ///< The model which transforms the measurement of state into state adjustments
            PROCESS_NOISE,      ///< The model which regulates the noise of the physical process
            MEASUREMENT_NOISE,  ///< The model which regulates the noise of the precision of measurement
            INITIAL_STATE       ///< The starting state of the process
        };
        /** Returns a reference to an internal model */
        matrix& model(models m) noexcept(false);
        /** Produces the next prediction model based on the inputs
         * \param [in] u The inputs to the system
         */
        matrix prediction(matrix &u);
        /** Produces the next prediction model based on the inputs
         * \param [in] inputs The inputs to the system
         */
        matrix prediction(matrix *inputs);
        /** Produces the corrected prediction of the model based on the measurements */
        matrix correction(matrix &z);
        /** Produces the corrected prediction of the model based on the measurements */
        matrix correction(matrix *measurements);

    };
}
