#pragma once

#include <opencv2/core.hpp>

namespace rv {
    void drawAxis(cv::Mat& image, double size, cv::Mat cameraMatrix, cv::Mat distortion, cv::Mat rvc, cv::Mat tvec);
}