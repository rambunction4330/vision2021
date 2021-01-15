#pragma once

#include <vector>

#include <opencv2/core.hpp>

namespace rv {
  template<typename _OutType, typename _InType>
  std::vector<cv::Point_<_OutType>> convertToPoints(std::vector<cv::Point_<_InType>>& input) {
    std::vector<cv::Point_<_OutType>> output;
    std::transform(input.begin(), input.end(), output.begin(), [](const cv::Point_<_InType>& p) {
      return cv::Point_<_OutType>(static_cast<_OutType>(p.x), static_cast<_OutType>(p.y));
    });
    return output;
  }

  template<typename _OutType, typename _InType>
  std::vector<cv::Point_<_OutType>> convertToPoints(std::vector<cv::Point3_<_InType>>& input) {
    std::vector<cv::Point_<_OutType>> output;
    std::transform(input.begin(), input.end(), output.begin(), [](const cv::Point3_<_InType>& p) {
      return cv::Point_<_OutType>(static_cast<_OutType>(p.x), static_cast<_OutType>(p.y));
    });
    return output;
  }

  template<typename _OutType, typename _InType>
  std::vector<cv::Point3_<_OutType>> convertToPoints3(std::vector<cv::Point_<_InType>>& input) {
    std::vector<cv::Point3_<_OutType>> output;
    std::transform(input.begin(), input.end(), output.begin(), [](const cv::Point_<_InType>& p) {
      return cv::Point3_<_OutType>(static_cast<_OutType>(p.x), static_cast<_OutType>(p.y), 0);
    });
    return output;
  }

  template<typename _OutType, typename _InType>
  std::vector<cv::Point3_<_OutType>> convertToPoints3(std::vector<cv::Point3_<_InType>>& input) {
    std::vector<cv::Point3_<_OutType>> output;
    std::transform(input.begin(), input.end(), output.begin(), [](const cv::Point3_<_InType>& p) {
      return cv::Point3_<_OutType>(static_cast<_OutType>(p.x), static_cast<_OutType>(p.y), static_cast<_OutType>(p.z));
    });
    return output;
  }

  template<typename _Type>
  std::vector<_Type> inVector(_Type value) { return std::vector<_Type> {value}; }
}