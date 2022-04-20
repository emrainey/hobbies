#include <opencv2/opencv.hpp>

// forward declaration
template <typename TYPE>
static void on_trackbar_callback(int value, void *instance);

namespace linalg {  // will be part of linalg-utils eventually

template <typename TYPE>
class Trackbar {
public:
    Trackbar(std::string my_name, std::string parent, TYPE min, TYPE def, TYPE max, TYPE step, TYPE *update = nullptr)
        : m_parent{parent}
        , m_my_name{my_name}
        , m_step{step}
        , m_min{min}
        , m_def{def}
        , m_max{max}
        , m_value{def}
        , m_update{update}
        , m_gradations{(max - min) / step} {
        int default_value = (m_def - m_min) / m_step;
        cv::createTrackbar(m_my_name, m_parent, nullptr, m_gradations, on_trackbar_callback<TYPE>,
                           reinterpret_cast<void *>(this));
        int minPos = 0, maxPos = m_gradations;
        cv::setTrackbarMin(m_my_name, m_parent, minPos);
        cv::setTrackbarMax(m_my_name, m_parent, maxPos);
        cv::setTrackbarPos(m_my_name, m_parent, default_value);
        callback(default_value);
    }

    void callback(int value) {
        m_value = (double(value) * m_step) + m_min;
        if (m_update) {
            *m_update = m_value;
        }
    }

    TYPE value() const {
        return m_value;
    }

    void set(TYPE value) {
        if (m_min <= value and value <= m_max) {
            m_value = value;
            int pos = int((m_value - m_min) / m_step);
            cv::setTrackbarPos(m_my_name, m_parent, pos);
        }
    }

    TYPE get() {
        int value = cv::getTrackbarPos(m_my_name, m_parent);
        m_value = (TYPE(value) * m_step) + m_min;
        return m_value;
    }

private:
    std::string m_parent;
    std::string m_my_name;
    TYPE m_step;
    TYPE m_min;
    TYPE m_def;
    TYPE m_max;
    TYPE m_value;
    TYPE *m_update;
    size_t m_gradations;
};

}  // namespace linalg

template <typename TYPE>
static void on_trackbar_callback(int value, void *instance) {
    linalg::Trackbar<TYPE> *trackbar = reinterpret_cast<linalg::Trackbar<TYPE> *>(instance);
    if (trackbar) {
        trackbar->callback(value);
    }
}
