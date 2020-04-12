#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <chrono>

#include "src/slic.hpp"

static cv::Mat1b createContourMask(const cv::Mat& labels)
{
    CV_Assert(labels.type() == CV_32S);

    const int rows = labels.rows;
    const int cols = labels.cols;
    cv::Mat dst(rows, cols, CV_8U, cv::Scalar::all(0));

    dst.forEach<uchar>([&](uchar& v, const int* p) {
        const int x = p[1];
        const int y = p[0];

        const int c = labels.ptr<int>(y)[x];
        const int r = x + 1 < cols ? labels.ptr<int>(y)[x + 1] : -1;
        const int d = y + 1 < rows ? labels.ptr<int>(y + 1)[x] : -1;

        if (r >= 0 && c != r) v = 255u;
        if (d >= 0 && c != d) v = 255u;
    });

    return dst;
}

static cv::Mat3b createSuperpixelImage(const cv::Mat& image, const cv::Mat& labels)
{
    CV_Assert(image.type() == CV_8UC3);
    CV_Assert(labels.type() == CV_32S);
    CV_Assert(image.size() == labels.size());

    const int rows = image.rows;
    const int cols = image.cols;
    cv::Mat dst(rows, cols, CV_8UC3);

    double maxLabel;
    cv::minMaxLoc(labels, NULL, &maxLabel);

    std::vector<cv::Vec3i> colors(static_cast<int>(maxLabel) + 1);
    std::vector<int> sizes(static_cast<int>(maxLabel) + 1, 0);

    for (int y = 0; y != rows; ++y) {

        const int* _label = labels.ptr<int>(y);
        const cv::Vec3b* _image = image.ptr<cv::Vec3b>(y);

        for (int x = 0; x != cols; ++x) {
            colors[_label[x]] += _image[x];
            sizes[_label[x]]++;
        }
    }

    for (int i = 0; i != colors.size(); ++i)
        if (sizes[i] != 0)
            colors[i] /= sizes[i];

    dst.forEach<cv::Vec3b>([&](cv::Vec3b& v, const int* p) {
        v = colors[labels.ptr<int>(p[0])[p[1]]];
    });

    return dst;
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cout << "Usage: ./slic.exe [image]" << std::endl;
        exit(EXIT_FAILURE);
    }

    cv::Mat image = cv::imread(argv[1]);

    const int superpixel_size = 30;
    const int iterate = 10;
    const float m = 20.f;

    // set SLIC parameters
    SLIC::Parameters param;
    param.superpixel_size = superpixel_size;
    param.iterate = 10;
    param.color_scale = m;
    // create SLIC
    SLIC slic(param);

    double time = 0;
    for (int i = 0; i <= 100; i++) {
        const auto t1 = std::chrono::system_clock::now();
        // perform SLIC
        slic.apply(image);
        const auto t2 = std::chrono::system_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (i > 0) time += duration;
    }

    std::printf("size: %d x %d\n", image.rows, image.cols);
    std::printf("elapsed: %.1f[msec]\n", time / 100);

    cv::Mat labels;
    slic.getLabels(labels);

    cv::imshow("Superpixel image", createSuperpixelImage(image, labels));
    image.setTo(cv::Scalar(0, 0, 255), createContourMask(labels));
    cv::imshow("Superpixel contour", image);
    cv::waitKey(0);

    return 0;
}
