#pragma once

#include <vector>

#include <opencv2/core.hpp>

namespace rv {
  template<typename _OutType, typename _InType>
  std::vector<cv::Point_<_OutType>> convertToPoints(std::vector<cv::Point_<_InType>>& input);

  template<typename _OutType, typename _InType>
  std::vector<cv::Point_<_OutType>> convertToPoints(std::vector<cv::Point3_<_InType>>& input);

  template<typename _OutType, typename _InType>
  std::vector<cv::Point3_<_OutType>> convertToPoints3(std::vector<cv::Point_<_InType>>& input);

  template<typename _OutType, typename _InType>
  std::vector<cv::Point3_<_OutType>> convertToPoints3(std::vector<cv::Point3_<_InType>>& input);

  template<typename _Type>
  std::vector<_Type> inVector(_Type value) { return std::vector<_Type> {value}; }
}