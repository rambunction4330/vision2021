#include "rambunctionVision/imageProcessing.hpp"

#include <vector>
#include <filesystem>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

namespace rv {
  void thresholdImage(cv::Mat& src, cv::Mat& dst, rv::Threshold threshold) {
    cv::Mat blur, hsv, thresh, open, close;

    // Mean blur over the image to remove noise
    cv::blur(src, blur, cv::Size(std::max(threshold.blurSize, 1), std::max(threshold.blurSize, 1)));

    // Convert to hsv color spave amd threshold
    cv::cvtColor(blur, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(threshold.low), cv::Scalar(threshold.high), thresh);

    // Use morphology to close any holes, and remove any extra noise
    cv::morphologyEx(thresh, open, cv::MORPH_OPEN, threshold.openMatrix);
    cv::morphologyEx(open, close, cv::MORPH_CLOSE, threshold.closeMatrix);

    // Output the fila image
    dst = close;
  }

  bool extractImagesFromDirectory(std::string filepath, std::vector<cv::Mat>& images) {
    // Double check that the directory exists.
    if (std::filesystem::exists(filepath)) {
      return false;
    }

    // Iterate through the files trying to turn them to images.
    for (auto& file : std::filesystem::directory_iterator(filepath)) {
      cv::Mat image = cv::imread(file.path().string());

      // If OpenCV could lpoad in the file as an image, add them to the vector.
      if (!image.empty()) {
        images.push_back(image);
      }
    }

    return true;
  }
}