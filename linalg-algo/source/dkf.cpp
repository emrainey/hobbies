
#include "linalg/dkf.hpp"

namespace linalg {
    dkf::dkf(size_t num_variables_in_state, size_t num_variables_in_input, size_t num_variables_in_measurement) :
        ns(num_variables_in_state), ni(num_variables_in_input), nm(num_variables_in_measurement),
        x{{ns,1},{ns,1}}, A(ns, ns), B(ns, ni), H(nm, ns), K(ns,nm), Q (ns, ns), R(nm, nm), I(ns, ns), P{{ns,ns},{ns,ns}} {
        fprintf(stderr, "Warning: Untested!\n");
        A = matrix::identity(ns, ns);
        B = matrix::identity(ns, ni);
        H = matrix::identity(nm, ns);
        K = matrix::identity(ns, nm);
        Q = matrix::identity(ns, ns);
        R = matrix::identity(nm, nm);
        I = matrix::identity(ns, ns);
        P[k_1] = matrix::identity(ns, ns);
        P[k] = matrix::identity(ns, ns);
    }

    dkf::~dkf() {}

    matrix &dkf::model(models m) noexcept(false) {
        switch (m) {
            case STATE_TRANSITION:  return A;
            case INPUT_TRANSLATION: return B;
            case STATE_MEASUREMENT: return H;
            case PROCESS_NOISE:     return Q;
            case MEASUREMENT_NOISE: return R;
            case INITIAL_STATE:     return x[k_1];
        }
        throw basal::exception("Must only supply a model enumeration", __FILE__, __LINE__);
    }

    matrix dkf::prediction(matrix& u) {
        basal::exception::throw_unless(u.rows == ni && u.cols == 1, __FILE__, __LINE__, "Inputs must have the dimensions described in constructor, inputs x 1");
        x[k] = A * x[k_1] + B * u;
        P[k] = (A * P[k_1]) * (A^T) + Q;
        return x[k];
    }

    matrix dkf::prediction(matrix* inputs) {
        x[k] = A * x[k_1];
        if (inputs) {
            matrix u = (*inputs);
            basal::exception::throw_unless(u.rows == ni && u.cols == 1, __FILE__, __LINE__, "Inputs must have the dimensions described in constructor, inputs x 1");
            x[k] += B * u;
        }
        P[k] = (A * P[k_1]) * (A^T) + Q;
        return x[k];
    }

    matrix dkf::correction(matrix& z) {
        basal::exception::throw_unless(z.rows == nm && z.cols == 1, __FILE__, __LINE__, "Measurements must have the dimensions described in the constructor, measurements x 1");
        K = (P[k] * (H^T)) / ((H * P[k]) * (H^T) + R);
        x[k_1] = x[k] + K * (z - H * x[k]);
        P[k_1] = (I - (K * H)) * P[k];
        return x[k_1];
    }

    matrix dkf::correction(matrix* measurements) {
        basal::exception::throw_if(measurements == nullptr, __FILE__, __LINE__, "Measurements can not be nullptr");
        matrix z = *measurements;
        basal::exception::throw_unless(z.rows == nm && z.cols == 1, __FILE__, __LINE__, "Measurements must have the dimensions described in the constructor, measurements x 1");
        matrix PkHT = P[k] * (H^T);
        matrix HPkHTR = ((H * PkHT) + R);
        K = PkHT / HPkHTR;
        // the residual is the z - H*x[k] term
        x[k_1] = x[k] + K * (z - H * x[k]);
        P[k_1] = (I - K * H) * P[k];
        return x[k_1];
    }
}
