#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc.hpp>
#include <iostream>
#include <chrono>

#include "../common/debug.hpp"
#include "slic.hpp"

void bench_OpenCV(const cv::Mat& image, int size = 30, float m = 20.0f, int iteration = 10)
{
    auto slic = cv::ximgproc::createSuperpixelSLIC(image, cv::ximgproc::SLIC, size, m);

    double time = 0;
    for (int i = 0; i <= iteration; i++) {
        const auto t1 = std::chrono::system_clock::now();
        slic->iterate(10);
        const auto t2 = std::chrono::system_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (i > 0)
            time += duration;
    }
    
    std::printf("OpenCV\n");
    std::printf("size: %d x %d\n", image.rows, image.cols);
    std::printf("elapsed: %.1f[msec]\n", time / iteration);

    cv::Mat label, label_color, label_contour;
    slic->getLabels(label);

    drawLabelColor(label, label_color);
    drawLabelContour(image, label, label_contour);

    //cv::imshow("OpenCV label", label_color);
    cv::imshow("OpenCV contour", label_contour);
    cv::waitKey(1);
}

void bench_CPU(const cv::Mat& image, int size = 30, float m = 20.0f, int iteration = 10)
{
    superpixel::SLIC::Parameters param;
    param.superpixel_size = size;
    param.iterate = 10;
    param.color_scale = m;
    superpixel::SLIC slic(param);
    
    double time = 0;
    for (int i = 0; i <= iteration; i++) {
        const auto t1 = std::chrono::system_clock::now();
        slic.apply(image);
        const auto t2 = std::chrono::system_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (i > 0)
            time += duration;
    }

    std::printf("CPU\n");
    std::printf("size: %d x %d\n", image.rows, image.cols);
    std::printf("elapsed: %.1f[msec]\n", time / iteration);

    cv::Mat label, label_color, label_contour;
    slic.getLabels(label);

    drawLabelColor(label, label_color);
    drawLabelContour(image, label, label_contour);

    //cv::imshow("CPU label", label_color);
    cv::imshow("CPU contour", label_contour);
    cv::imwrite("SLIC_label_contour.png", label_contour);
    cv::waitKey(1);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        exit(EXIT_FAILURE);
    }

    cv::Mat image = cv::imread(argv[1]);

    float m = 20.0f;
    int size = 30;

    bench_OpenCV(image, size, m, 10);
    bench_CPU(image, size, m, 10);

    cv::waitKey(0);

    return 0;
}


