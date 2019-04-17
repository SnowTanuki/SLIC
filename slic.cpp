#include "slic.hpp"
#include <cmath>
#include <climits>

namespace superpixel {

/****************/
/** Paremeters **/
/****************/
SLIC::Parameters::Parameters(int superpixel_size, int iterate, float color_scale) 
: superpixel_size(superpixel_size), iterate(iterate), color_scale(color_scale) {
    if (this->superpixel_size <= 0) this->superpixel_size = 30;
    if (this->iterate <= 0) this->iterate = 10;
    if (this->color_scale <= 0) this->color_scale = 20.0f;
}

/*******************/
/** ClusterCenter **/
/*******************/
SLIC::ClusterCenter::ClusterCenter() : x(0), y(0), l(0), a(0), b(0) {}
SLIC::ClusterCenter::ClusterCenter(int _x, int _y, cv::Vec3b _lab) : x(_x), y(_y), l(_lab[0]), a(_lab[1]), b(_lab[2]) {}
SLIC::ClusterCenter::ClusterCenter(int _x, int _y, uchar _l, uchar _a, uchar _b) : x(_x), y(_y), l(_l), a(_a), b(_b) {}

SLIC::ClusterCenter SLIC::ClusterCenter::operator / (int n) const {
    return ClusterCenter(this->x / n, this->y / n, this->l / n, this->a / n, this->b / n);
}

SLIC::ClusterCenter& SLIC::ClusterCenter::operator = (const ClusterCenter& obj) {
    this->x = obj.x, this->y = obj.y, this->l = obj.l, this->a = obj.a, this->b = obj.b;
    return *this;
}

SLIC::ClusterCenter& SLIC::ClusterCenter::operator += (const ClusterCenter& obj) {
    this->x += obj.x, this->y += obj.y, this->l += obj.l, this->a += obj.a, this->b += obj.b;
    return *this;
}

inline void SLIC::ClusterCenter::set(int _x, int _y, cv::Vec3b _lab) {
    this->x = _x, this->y = _y, this->l = _lab[0], this->a = _lab[1], this->b = _lab[2];
}

/**********/
/** SLIC **/
/**********/
SLIC::SLIC(const Parameters& param) : param_(param) {}

SLIC::~SLIC() {
    lab_.release();
    labels_.release();
    distance_.release();
    centers_.shrink_to_fit();
}

inline void SLIC::init(const cv::Mat& image) {

    rows_ = image.rows;
    cols_ = image.cols;

    cv::cvtColor(image, lab_, cv::COLOR_BGR2Lab);
    labels_.create(rows_, cols_);
    labels_.setTo(cv::Scalar::all(-1));
    distance_.create(rows_, cols_);
    distance_.setTo(cv::Scalar::all(1e7f));

    if (!centers_.empty()) centers_.clear();

    const int init_size = param_.superpixel_size;
    const int ofs = param_.superpixel_size / 2;
    num_superpixels_ = 0;

    for (int y = ofs; y < rows_; y += init_size) {

        const cv::Vec3b* _lab = lab_.ptr<cv::Vec3b>(y);

        for (int x = ofs; x < cols_; x += init_size) {
            centers_.push_back(ClusterCenter(x, y, _lab[x]));
            num_superpixels_++;
        }
    }

    centers_tmp_ = std::vector<ClusterCenter>(num_superpixels_);
    centers_distance_ = std::vector<int>(num_superpixels_);

    S_ = (int)std::ceilf(std::sqrtf(rows_ * cols_ / (float)num_superpixels_));

    color_scale_norm_ = 1.0f / (param_.color_scale * param_.color_scale);
    S_norm_ = 1.0f / (S_ * S_);

    cv::Mat gradient_image_;
    cv::Laplacian(lab_, gradient_image_, CV_32F, 1);

    for (auto& center : centers_) {

        const int x = center.x;
        const int y = center.y;

        cv::Vec3f _center_grad = gradient_image_.ptr<cv::Vec3f>(y)[x];
        float min_grad = _center_grad[0] + _center_grad[1] + _center_grad[2];
        int min_grad_x = x;
        int min_grad_y = y;

        const int xs = std::max(x - 1, 0);
        const int xe = std::min(x + 2, cols_);
        const int ys = std::max(y - 1, 0);
        const int ye = std::min(y + 2, rows_);

        for (int yj = ys; yj != ye; yj++) {

            const cv::Vec3f* _grad_img = gradient_image_.ptr<cv::Vec3f>(yj);

            for (int xi = xs; xi != xe; xi++) {

                const float grad = _grad_img[xi][0] + _grad_img[xi][1] + _grad_img[xi][2];

                if (min_grad > grad) {
                    min_grad = grad;
                    min_grad_x = xi;
                    min_grad_y = yj;
                }
            }
        }

        center.set(min_grad_x, min_grad_y, lab_.ptr<cv::Vec3b>(min_grad_y)[min_grad_x]);
    }
}

inline float SLIC::getDistance(ClusterCenter& center, int x, int y) const {

    const cv::Vec3b p_color = lab_.ptr<cv::Vec3b>(y)[x];

    const int ldiff = center.l - p_color[0];
    const int adiff = center.a - p_color[1];
    const int bdiff = center.b - p_color[2];
    const int xdiff = center.x - x;
    const int ydiff = center.y - y;

    return color_scale_norm_ * (ldiff * ldiff + adiff * adiff + bdiff * bdiff) + S_norm_ * (xdiff * xdiff + ydiff * ydiff);
}

inline void SLIC::updateCenters() {

    for_each(centers_distance_.begin(), centers_distance_.end(), [](int& d) { d = INT_MAX; });

    for (int y = 0; y != rows_; ++y) {

        const int* _label = labels_.ptr<int>(y);
        const cv::Vec3b* _lab = lab_.ptr<cv::Vec3b>(y);

        for (int x = 0; x != cols_; ++x) {

            const int l = _label[x];
            const int ldiff = centers_tmp_[l].l - _lab[x][0];
            const int adiff = centers_tmp_[l].a - _lab[x][1];
            const int bdiff = centers_tmp_[l].b - _lab[x][2];
            const int dist = ldiff * ldiff + adiff * adiff + bdiff * bdiff;

            if (centers_distance_[l] > dist) {
                centers_distance_[l] = dist;
                centers_[l].set(x, y, _lab[x]);
            }
        }
    }
}

inline int SLIC::iterate() {

    int num_updated = 0;

    for (int center_num = 0; center_num != num_superpixels_; ++center_num) {

        ClusterCenter center = centers_[center_num];
        ClusterCenter new_center;
        int count = 0;

        const int xs = std::max(center.x - S_, 0);
        const int xe = std::min(center.x + S_ + 1, cols_);
        const int ys = std::max(center.y - S_, 0);
        const int ye = std::min(center.y + S_ + 1, rows_);

        for (int y = ys; y != ye; ++y) {

            float* _dist = distance_.ptr<float>(y);
            int* _label = labels_.ptr<int>(y);
            const cv::Vec3b* _lab = lab_.ptr<cv::Vec3b>(y);

            for (int x = xs; x != xe; ++x) {

                const float dist = getDistance(center, x, y);

                if (_dist[x] > dist) {
                    _dist[x] = dist;
                    _label[x] = center_num;
                    num_updated++;
                }

                if (_label[x] == center_num) {
                    new_center += ClusterCenter(x, y, _lab[x]);
                    count++;
                }
            }
        }

        centers_tmp_[center_num] = new_center / count;
    }

    updateCenters();

    return num_updated;
}

void SLIC::apply(const cv::Mat& image) {

    CV_Assert(image.type() == CV_8UC3);
    init(image);

    int itr = 0;
    while (itr++ != param_.iterate && iterate() != 0) {}
}

void SLIC::getLabels(cv::Mat& label_out) const {
    labels_.copyTo(label_out);
}

}