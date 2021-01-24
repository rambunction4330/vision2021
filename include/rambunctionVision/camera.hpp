/**
 * @file camera.hpp
 * @author George Jurgiel (gcjurgiel@icloud.com)
 * @brief Structer and functions to hold and change camera settings
 * @version 0.1
 * @date 2021-01-23
 * 
 * @copyright Copyright (c) 2021
 */
#include <opencv2/core.hpp>

/**
 * @brief 'Rambunction Vision' namespace to store shared code.
 */
namespace rv {

  /**
   * @brief The camera properties.
   */
  struct Camera {
    cv::Mat matrix; /**< The intrensic camera matrix to convert between 2d and 3d points. */
    cv::Mat distortion; /**< The coeeficents to account for lense distortion. */

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