#ifndef __SLIC_HPP__
#define __SLIC_HPP__

#include <opencv2/opencv.hpp>
#include <vector>

namespace superpixel {

class SLIC {

public:
    struct Parameters {
        int superpixel_size;
        int iterate;
        float color_scale;
        Parameters(int superpixel_size = 20, int iterate = 10, float color_scale = 30.0f);
    };

    SLIC(const Parameters& param);
    ~SLIC();
    void apply(const cv::Mat& image);
    void getLabels(cv::Mat& label_out) const;

private:
    struct ClusterCenter {
        int x, y;
        int l, a, b;
        ClusterCenter();
        ClusterCenter(int _x, int _y, cv::Vec3b _lab);
        ClusterCenter(int _x, int _y, uchar _l, uchar _a, uchar _b);

        ClusterCenter operator / (int n) const;
        ClusterCenter& operator = (const ClusterCenter& obj);
        ClusterCenter& operator += (const ClusterCenter& obj);

        inline void set(int _x, int _y, cv::Vec3b _lab);
    };

    cv::Mat lab_;
    cv::Mat_<int> labels_;
    cv::Mat_<float> distance_;
    std::vector<ClusterCenter> centers_;
    std::vector<ClusterCenter> centers_tmp_;
    std::vector<int> centers_distance_;

    const Parameters param_;

    int S_;
    float color_scale_norm_, S_norm_;
    int num_superpixels_;
    int rows_, cols_;

    inline void init(const cv::Mat& image);
    inline float getDistance(ClusterCenter& center, int x, int y) const;
    inline void updateCenters();
    inline int iterate();
};

}

#endif
