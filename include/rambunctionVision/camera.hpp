#include <opencv2/core.hpp>

namespace rv {
  struct Camera {
    cv::Mat matrix;
    cv::Mat distortion;

    void write(cv::FileStorage& fs) const {
      fs << "{" << "Matrix" << matrix << "Distortion" << distortion << "}";
    }

    void read(const cv::FileNode& node) {
      node["Matrix"] >> matrix;
      node["Distortion"] >> distortion;
    }
  };

  static void write(cv::FileStorage& fs, const std::string&, const rv::Camera& x) { 
    x.write(fs);
  }
  
  static void read(const cv::FileNode& node, rv::Camera& x, const rv::Camera& default_value = rv::Camera()){
    if(node.empty()) {
      x = default_value;
    } else {
      x.read(node);
    }
  }
}