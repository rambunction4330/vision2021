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

    void setHighH(int v) { highH() = std::clamp(v, lowH() + 1, 180); }
    void setLowH(int v) { lowH() = std::clamp(v, 0, highH() - 1); }
    void setHighS(int v) { highV() = std::clamp(v, lowS() + 1, 255); }
    void setLowS(int v) { lowV() = std::clamp(v, 0, highS() - 1); }
    void setHighV(int v) { highV() = std::clamp(v, lowV() + 1, 255); }
    void setLowV(int v) { lowV() = std::clamp(v, 0, highV() - 1); }
  };

  void thresholdImage(cv::Mat& src, cv::Mat& dst, int blurSize, rv::Threshold threshold, cv::Mat open, cv::Mat close);
}