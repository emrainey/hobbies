#include <opencv2/opencv.hpp>

namespace linalg {
template <typename TYPE>
class Trackbar {
public:
    Trackbar(std::string parent, std::string my_name, TYPE min, TYPE def, TYPE max, size_t gradations)
        : m_parent(parent)
        , m_my_name(my_name)
        . m_min(min)
        , m_def(def)
        , m_max(def)
        {
            m_scalar = (max - min) / gradations;
            int default_value = m_def * m_scalar;
            cv::createTrackbar(m_my_name, m_parent, &default_value, gradations, on_trackbar_callback, reinterpret_cast<void*>(this));
            //cv::setTrackbarMin(m_my_name, m_parent, m_min);
    }

    void callback(int value) {
        m_value = double(value) / m_scalar;
    }

    TYPE value() const {
        return m_value;
    }

    void set(TYPE value) {
        if (m_min <= value and value <= m_max) {
            m_value = value;
            int pos = int(m_value * m_scalar);
            cv::setTrackbarPos(m_my_name, m_parent, pos);
        }
    }

    TYPE get() {
        int value = cv::getTrackbarPos(m_my_name, m_parent);
        m_value = TYPE(value) / m_scalar;
        return m_value;
    }

private:
    std::string m_parent;
    std::string m_my_name;
    TYPE m_scalar;
    TYPE m_min;
    TYPE m_def;
    TYPE m_max;
    TYPE m_value;
};

} // namespace linalg

void on_trackbar_callback(int value, void * instance) {
    Trackbar * trackbar = reinterpret_cast<Trackbar *>(instance);
    trackbar->callback(value);
}
