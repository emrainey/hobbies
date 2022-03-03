
#include "neuralnet/network.hpp"

#include <algorithm>
#include <linalg/utils.hpp>

namespace nn {

std::vector<size_t> convert(std::initializer_list<int> list) {
    std::vector<size_t> v;
    v.insert(v.end(), list.begin(), list.end());
    return v;
}

network::network() : layers() {
}

network::network(std::initializer_list<size_t> list) : layers() {
    std::vector<size_t> v = list;
    create(v);
}

network::network(std::vector<size_t> list) : layers() {
    create(list);
}

void network::create(std::vector<size_t> list) {
    for (size_t i = 0; i < list.size(); i++) {
        if (i == 0) {  // first element
            printf("Creating input of %zu nodes\n", list[i]);
            layers.push_back(new input(list[i]));
        } else if (i == (list.size() - 1)) {  // last element
            printf("Creating output of %zu nodes\n", list[i]);
            layers.push_back(new output(list[i - 1], list[i]));
        } else {
            printf("Creating hidden of %zu nodes\n", list[i]);
            layers.push_back(new hidden(list[i - 1], list[i]));
        }
    }
}

network::~network() {
    for (auto& ptr : layers) {
        if (ptr->isa(layer::type::input)) {
            input* tmp = dynamic_cast<input*>(ptr);
            delete tmp;
        } else if (ptr->isa(layer::type::hidden)) {
            hidden* tmp = dynamic_cast<hidden*>(ptr);
            delete tmp;
        } else if (ptr->isa(layer::type::output)) {
            output* tmp = dynamic_cast<output*>(ptr);
            delete tmp;
        }
    }
}

void network::forward() {
    int index = -1;
    for_each([&](layer& _layer) {
        if (_layer.isa(layer::type::input)) {
            // printf("i");
            _layer.forward();
        } else if (_layer.isa(layer::type::hidden)) {
            // printf("h");
            _layer.forward(get(index));
        } else if (_layer.isa(layer::type::output)) {
            // printf("o");
            _layer.forward(get(index));
        }
        index++;
    });
    // printf(">");
}

void network::backward(double alpha, double gamma) {
    basal::exception::throw_unless(layers.size() > 2, __FILE__, __LINE__);
    for (int index = layers.size() - 1; index >= 0; index--) {
        layer& _layer = (*layers[index]);
        if (_layer.isa(layer::type::input)) {
            _layer.backward();
        } else if (_layer.isa(layer::type::hidden)) {
            _layer.backward(get(index - 1), alpha, gamma);
        } else if (_layer.isa(layer::type::output)) {
            _layer.backward(get(index - 1), alpha, gamma);
        }
    }
}

network& network::operator<<(layer& lay) {
    layers.push_back(&lay);
    return (*this);
}

network& network::operator<<(layer&& lay) {  // r-value or move
    layers.push_back(&lay);
    return (*this);
}

layer& network::get(size_t i) {
    return *(layers[i]);
}

network& network::for_each(std::function<void(layer& l)> block) {
    for (auto& ptr : layers) {
        block(*ptr);
    }
    return (*this);
}

void network::set(activation_type type) {
    for_each([&](layer& _layer) -> void {
        if (_layer.isa(layer::type::input)) {
            return;
        }
        inner& in = dynamic_cast<inner&>(_layer);
        in.set(type);
    });
}

void network::update(void) {
    for_each([](layer& _layer) {
        if (_layer.isa(layer::type::input)) {
            return;
        }
        inner& in = dynamic_cast<inner&>(_layer);
        in.update();
    });
}

void network::reset(void) {
    for_each([](layer& _layer) {
        if (_layer.isa(layer::type::input)) {
            return;
        }
        inner& in = dynamic_cast<inner&>(_layer);
        in.reset();
    });
}

static cv::Scalar black(0);

static void print(cv::Mat& img, cv::Point& pt, const char fmt[], ...) {
    char buffer[256] = {0};
    va_list list;
    va_start(list, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, list);
    va_end(list);
    double scl = 0.5;
    int thickness = 1;
    int baseline = 0;
    int fontFace = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
    std::string str(buffer);
    cv::Size sz = cv::getTextSize(str, fontFace, scl, thickness, &baseline);
    baseline += thickness;  // why?
    cv::Point textOrg((img.cols - sz.width) / 2, (img.rows + sz.height) / 2);
    cv::putText(img, str, pt, fontFace, scl, black, thickness, 8, false);
}

static cv::Size compute_complete_dimensions(std::vector<std::vector<cv::Mat>>& images,
                                            std::vector<std::vector<cv::Rect>>& rects, int min_height, int border) {
    int ix = 0;
    int width = 0;
    int height = min_height - 2 * border;
    for (auto& set : images) {
        for (auto& img : set) {
            height = std::max(img.rows, height);
        }
    }
    // height is now the max height of any particular image or min_height, which ever is larger.
    int scale_to = height - 2 * border;
    int hy = height / 2;
    for (size_t lidx = 0; lidx < images.size(); lidx++) {
        for (size_t idx = 0; idx < images[lidx].size(); idx++) {
            cv::Mat& img = images[lidx][idx];
            ix += border;
            // will scale this up to to fit the exact scale_to
            if (img.rows < scale_to and img.cols == 1) {
                int scale_factor = (int)std::round((float)scale_to / img.rows);
                rects[lidx].push_back(cv::Rect(ix, border, scale_factor, scale_to));
            } else if (img.rows == img.cols) {
                // square weights should scale up to a integer multiple under or at the scale_to
                int scale_factor = (int)std::round((float)scale_to / img.rows);
                rects[lidx].push_back(cv::Rect(ix, border, scale_factor * img.cols, scale_factor * img.rows));
            } else if (img.rows > 1 and img.cols > 1) {
                // rectangular
                int scale_factor = (int)std::round((float)scale_to / img.rows);
                rects[lidx].push_back(cv::Rect(ix, border, scale_factor * img.cols, scale_factor * img.rows));
            } else {
                // just the normal sizes
                rects[lidx].push_back(
                    cv::Rect(ix, hy - images[lidx][idx].rows / 2, images[lidx][idx].cols, images[lidx][idx].rows));
            }
            // std::cout << "Rect[" << lidx << "][" << idx << "]=" << rects[lidx][idx] << std::endl;
            width += rects[lidx][idx].width + 2 * border;
            ix += rects[lidx][idx].width + border;
        }
    }
    cv::Size sz(width, height + 2 * border);
    // now verify that all rectangles fit in the image.
    cv::Rect rect(cv::Point(0, 0), sz);
    for (size_t lidx = 0; lidx < images.size(); lidx++) {
        for (size_t idx = 0; idx < images[lidx].size(); idx++) {
            if (not rect.contains(rects[lidx][idx].tl()) or not rect.contains(rects[lidx][idx].br())) {
                printf("ERROR! Rect[%zu][%zu] is not contained!\n", lidx, idx);
            }
        }
    }
    return sz;
}

static void jetImage() {
    static bool showJetImage = false;
    if (showJetImage) return;
    showJetImage = true;
    double high = 1.5;
    double low = -high;
    int width = (2.0 * high) * 100;
    printf("Jet Image is %d across\n", width);
    linalg::matrix values(1, width);
    values.for_each([&](size_t y, size_t x, double& v) {
        y |= 0;
        v = ((double)x / width);  // 0.0 to 1.0
        v *= (2.0 * high);        // 0.0 to 3.0
        v += low;                 // -1.5 to 1.5
        // should go from lower to high
    });
    cv::Mat bar = linalg::convert(values, CV_8UC3);
    cv::Size tophalf(width * 3, 100);
    cv::Point btmRow(20, 150);
    cv::Mat scaled(200, width * 3, CV_8UC3);
    cv::Point origin;
    cv::Rect rect(origin, tophalf);
    scaled = cv::Scalar(255, 255, 255);
    cv::resize(bar, scaled(rect), rect.size(), 0.0, 0.0, cv::INTER_NEAREST);
    print(scaled, btmRow, "Jet Image Scale, %lf to %lf", low, high);
    cv::imshow("Color Scale", scaled);
}

void network::visualize(std::chrono::milliseconds delay) {
    jetImage();
    const char windowName[] = "Network";
    if (get(0).values.rows > 16) {
        std::vector<std::vector<std::string>> names(layers.size());
        std::vector<std::vector<cv::Rect>> rects(layers.size());  // the desired, not actual rectangles.
        std::vector<std::vector<cv::Mat>> images(layers.size());  // the array of arrays of images.
        for (size_t lidx = 0; lidx < layers.size(); lidx++) {
            if (layers[lidx]->isa(layer::type::input)) {
                nn::input& in = as_input(lidx);
                images[lidx].push_back(linalg::squarish(in.values, CV_8UC3));
                names[lidx].push_back("v");
            } else {
                nn::inner& in = as_inner(lidx);
                // weights are already square-ish
                images[lidx].push_back(linalg::each_row_square_tiled(in.weights, CV_8UC3, 4));
                names[lidx].push_back("W");
                images[lidx].push_back(linalg::each_row_square_tiled(in.delta_weights, CV_8UC3, 4));
                names[lidx].push_back("dW");
                images[lidx].push_back(linalg::convert(in.biases, CV_8UC3));
                names[lidx].push_back("b");
                images[lidx].push_back(linalg::convert(in.delta_biases, CV_8UC3));
                names[lidx].push_back("db");
                images[lidx].push_back(linalg::convert(in.zeta, CV_8UC3));
                names[lidx].push_back("z");
                images[lidx].push_back(linalg::convert(in.values, CV_8UC3));
                names[lidx].push_back("v");
                images[lidx].push_back(linalg::convert(in.beta, CV_8UC3));
                names[lidx].push_back("B");
                images[lidx].push_back(linalg::convert(in.delta, CV_8UC3));
                names[lidx].push_back("D");
                images[lidx].push_back(linalg::convert(in.rms, CV_8UC3));
                names[lidx].push_back("RMS");
            }
        }
        size_t border = 20;
        size_t min_height = 224 + 2 * border;
        cv::Size sz = compute_complete_dimensions(images, rects, min_height, border);
        // size_t width = sz.width;
        size_t height = sz.height;
        // std::cout << windowName << "=" << sz << std::endl;
        cv::Mat img(sz, CV_8UC3);
        // fill with white.
        img = cv::Scalar(255, 255, 255);
        for (size_t lidx = 0, ix = 0; lidx < images.size(); lidx++) {
            for (size_t idx = 0; idx < images[lidx].size(); idx++) {
                ix += border;
                cv::Point pt(ix, border / 2);
                print(img, pt, "%s", names[lidx][idx].c_str());
                cv::Size _sz = rects[lidx][idx].size();
                cv::resize(images[lidx][idx], img(rects[lidx][idx]), _sz, 0, 0, cv::INTER_NEAREST);
                ix += _sz.width + border;
            }
            // separator
            cv::Point top(ix, 0), btm(ix, height - 1);
            cv::line(img, top, btm, cv::Scalar(0, 0, 0), 1, 8, 0);
        }
        cv::imshow(windowName, img);
    } else {
        cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
        size_t scaling = 270;
        size_t width = 2 * scaling * layers.size();
        size_t height = 0;
        for_each([&](layer& _layer) { height = (height < _layer.values.rows ? _layer.values.rows : height); });
        height *= scaling;
        cv::Mat img(height, width, CV_8UC1);
        img = 255;  // white
        // organized as layers (outer) then columns node_ptr[l][n]
        std::vector<std::vector<cv::Point>> node_pts(layers.size());  // start with # layers
        size_t x = 0, y = 0;
        for (size_t l = 0; l < layers.size(); l++) {
            x = 2 * l * scaling + scaling / 2;
            for (size_t n = 0; n < layers[l]->values.rows; n++) {
                y = n * scaling + scaling / 2;
                cv::Point pt(x, y);
                node_pts[l].push_back(pt);
                cv::circle(img, pt, scaling / 3, black, 1, 8, 0);
                print(img, pt, "v=%0.3lf", layers[l]->values[n][0]);
                if (layers[l]->layer_type != layer::type::input) {
                    cv::Point b0 = pt + cv::Point(0, scaling / 3);
                    cv::Point B0 = pt + cv::Point(0, scaling / 6);
                    cv::Point z0 = pt - cv::Point(0, scaling / 6);
                    nn::inner& in = as_inner(l);
                    print(img, b0, "b=%0.3lf", in.biases[n][0]);
                    print(img, B0, "B=%0.3lf", in.beta[n][0]);
                    print(img, z0, "z=%0.3lf", in.zeta[n][0]);
                }
            }
        }
        for (size_t l = 1; l < node_pts.size(); l++) {
            for (size_t n = 0; n < node_pts[l].size(); n++) {
                for (size_t n_1 = 0; n_1 < node_pts[l - 1].size(); n_1++) {
                    cv::Point a = node_pts[l][n];
                    cv::Point b = node_pts[l - 1][n_1];
                    cv::line(img, a, b, black, 1, 8, 0);
                    cv::Point hf = a - (a - b) / 4;
                    cv::Point dw = hf + cv::Point(0, 20);
                    nn::inner& in = as_inner(l);
                    print(img, hf, "W=%0.3lf", in.weights[n][n_1]);
                    print(img, dw, "dw=%0.3lf", in.delta_weights[n][n_1]);
                }
            }
        }
        nn::output& out = as_output(node_pts.size() - 1);
        cv::Point br(width - scaling, height - 20);
        print(img, br, "RMS=%07lf", out.rms_value);
        cv::imshow(windowName, img);
    }
    int ms = delay.count();
    int key = cv::waitKey(ms);
    switch ((key & 0xFF)) {
        case 'q':
            exit(0);
            break;
        case ' ':
            break;
    }
}

input& network::as_input(size_t i) noexcept(false) {
    basal::exception::throw_unless(get(i).isa(layer::type::input), __FILE__, __LINE__);
    return dynamic_cast<input&>(get(i));
}

hidden& network::as_hidden(size_t i) noexcept(false) {
    basal::exception::throw_unless(get(i).isa(layer::type::hidden), __FILE__, __LINE__);
    return dynamic_cast<hidden&>(get(i));
}

output& network::as_output(size_t i) noexcept(false) {
    basal::exception::throw_unless(get(i).isa(layer::type::output), __FILE__, __LINE__);
    return dynamic_cast<output&>(get(i));
}

inner& network::as_inner(size_t i) noexcept(false) {
    basal::exception::throw_if(get(i).isa(layer::type::input), __FILE__, __LINE__);
    return dynamic_cast<inner&>(get(i));
}

}  // namespace nn
