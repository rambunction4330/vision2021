#pragma once

#include <opencv2/core.hpp>

namespace rv {
  struct Threshold {
    cv::Scalar_<int> high = {180, 255, 255};
    cv::Scalar_<int> low  = {0, 0, 0};

    int& highH() { return high[0]; }
    int& lowH()  { return low[0];  }
    int& highS() { return high[1]; }
    int& lowS()  { return low[1];  }
    int& highV() { return high[2]; }
    int& lowV()  { return low[2];  }

    void setHighH(int value) { highH() = std::clamp(value, lowH() + 1, 180); }
    void setLowH(int value) { lowH() = std::clamp(value, 0, highH() - 1); }
    void setHighS(int value) { highV() = std::clamp(value, lowS() + 1, 255); }
    void setLowS(int value) { lowV() = std::clamp(value, 0, highS() - 1); }
    void setHighV(int value) { highV() = std::clamp(value, lowV() + 1, 255); }
    void setLowV(int value) { lowV() = std::clamp(value, 0, highV() - 1); }
  };

  void thresholdImage(cv::Mat& src, cv::Mat& dst, int blurSize, rv::Threshold threshold, cv::Mat openMatrix, cv::Mat closeMatrix);

  void extractImagesFromDirectory(std::string filepath, std::vetcor<cv::Mat>& images);
}