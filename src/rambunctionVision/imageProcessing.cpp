#include "rambunctionVision/imageProcessing.hpp"

#include <vector>
#include <filesystem>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

namespace rv {
  void thresholdImage(cv::Mat& src, cv::Mat& dst, int blurSize, rv::Threshold threshold, cv::Mat openMatrix, cv::Mat closeMatrix) {
    cv::Mat blur, hsv, thresh, open, close;

    cv::blur(src, blur, cv::Size(blurSize, blurSize));

    cv::cvtColor(blur, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(threshold.low), cv::Scalar(threshold.high), thresh);

    cv::morphologyEx(thresh, open, cv::MORPH_OPEN, openMatrix);
    cv::morphologyEx(open, close, cv::MORPH_CLOSE, closeMatrix);

    dst = close;
  }

  bool extractImagesFromDirectory(std::string filepath, std::vector<cv::Mat>& images) {
    if (std::filesystem::exists(filepath)) {
      return false;
    }

    for (auto& file : std::filesystem::directory_iterator(filepath)) {
      cv::Mat image = cv::imread(file.path().string());

      if (!image.empty()) {
        images.push_back(image);
      }
    }

    return true;
  }
}