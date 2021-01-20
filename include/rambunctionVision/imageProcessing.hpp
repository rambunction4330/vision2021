#pragma once

#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace rv {
  struct Threshold {
    cv::Scalar_<int> high = {180, 255, 255};
    cv::Scalar_<int> low  = {0, 0, 0};

    int blurSize = 15;

    cv::Mat openMatrix = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15,15));
    cv::Mat closeMatrix = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15,15));

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

    void write(cv::FileStorage& fs) const {
      fs << "{" << "High" << high << "Low" << low << << "BlurSize" << blurSize << "OpenMatrix" << openMatrix << "CloseMatrix" << closeMatrix << "}";
    }

    void read(const cv::FileNode& node) {
      node["High"] >> high;
      node["Low"] >> low;
      node["BlurSize"] >> blurSize;
      node["OpenMatrix"] >> openMatrix;
      node["CloseMatrix"] >> closeMatrix;
    }
  };

  static void write(cv::FileStorage& fs, const std::string&, const rv::Threshold& x) { 
    x.write(fs);
  }
  
  static void read(const cv::FileNode& node, rv::Threshold& x, const rv::Threshold& default_value = rv::Threshold()){
    if(node.empty()) {
      x = default_value;
    } else {
      x.read(node);
    }
  }

  void thresholdImage(cv::Mat& src, cv::Mat& dst, rv::Threshold threshold);

  bool extractImagesFromDirectory(std::string filepath, std::vector<cv::Mat>& images);
}