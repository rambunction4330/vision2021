#include <rambunctionVision/imageProcessing.hpp>

#include <opencv2/imgproc.hpp>

namespace rv {
  void thresholdImage(cv::Mat& src, cv::Mat& dst, int blurSize, rv::Threshold threshold, cv::Mat& openMatrix, cv::Mat& closeMatrix) {
    cv::Mat norm, blur, hsv, thresh, open, close;

    cv::normalize(src, norm);
    cv::blur(norm, blur, cv::Size(blurSize, blurSize));

    cv::cvtColor(blur, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(threshold.low), cv::Scalar(threshold.high), thresh);

    cv::morphologyEx(thresh, open, cv::MORPH_OPEN, openMatrix);
    cv::morphologyEx(open, close, cv::MORPH_CLOSE, closeMatrix);

    dst = close;
  }
}