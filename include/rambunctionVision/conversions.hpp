/**
 * @file conversions.hpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2021-01-23
 * 
 * @copyright Copyright (c) 2021
 */

#pragma once

#include <vector>

#include <opencv2/core.hpp>

/**
 * @brief 'Rambunction Vision' namespace to store shared code.
 */
namespace rv {

  /**
   * @brief Converts a vector of 2 dimensional point to another type.
   * 
   * The output is identical to the input, just with a diffrent numeric type.
   * 
   * @tparam _OutType The numeric type of points to convert to.
   * @tparam _InType The numeric type that the points are being converted from.
   * @param[in] input The input vector of points to be converted to another type.
   * @return std::vector<cv::Point_<_OutType>> The output converted points.
   */
  template<typename _OutType, typename _InType>
  std::vector<cv::Point_<_OutType>> convertToPoints(std::vector<cv::Point_<_InType>>& input) {
    std::vector<cv::Point_<_OutType>> output(input.size());
    std::transform(input.begin(), input.end(), output.begin(), [](const cv::Point_<_InType>& p) {
      return cv::Point_<_OutType>(static_cast<_OutType>(p.x), static_cast<_OutType>(p.y));
    });
    return output;
  }

  /**
   * @brief Converts a vector of 3 dimensional point to another type.
   * 
   * The x and y parts of the output are identical to the input, just with a
   * diffrent numeric type. The z component is truncated.
   * 
   * @tparam _OutType The numeric type of points to convert to.
   * @tparam _InType The numeric type that the points are being converted from.
   * @param[in] input The input vector of points to be converted to another type.
   * @return std::vector<cv::Point_<_OutType>> The output converted points.
   */
  template<typename _OutType, typename _InType>
  std::vector<cv::Point_<_OutType>> convertToPoints(std::vector<cv::Point3_<_InType>>& input) {
    std::vector<cv::Point_<_OutType>> output(input.size());
    std::transform(input.begin(), input.end(), output.begin(), [](const cv::Point3_<_InType>& p) {
      return cv::Point_<_OutType>(static_cast<_OutType>(p.x), static_cast<_OutType>(p.y));
    });
    return output;
  }

  /**
   * @brief Converts a vector of 3 dimensional point to another type.
   * 
   * The x and y parts of the output are identical to the input, just with a
   * diffrent numeric type. The z component is set to 0.
   * 
   * @tparam _OutType The numeric type of points to convert to.
   * @tparam _InType The numeric type that the points are being converted from.
   * @param[in] input The input vector of points to be converted to another type.
   * @return std::vector<cv::Point3_<_OutType>> The output converted points.
   */
  template<typename _OutType, typename _InType>
  std::vector<cv::Point3_<_OutType>> convertToPoints3(std::vector<cv::Point_<_InType>>& input) {
    std::vector<cv::Point3_<_OutType>> output(input.size());
    std::transform(input.begin(), input.end(), output.begin(), [](const cv::Point_<_InType>& p) {
      return cv::Point3_<_OutType>(static_cast<_OutType>(p.x), static_cast<_OutType>(p.y), 0);
    });
    return output;
  }

  /**
   * @brief Converts a vector of 3 dimensional point to another type.
   * 
   * The output points are identical to int input in all but which numeric type they use.
   * 
   * @tparam _OutType The numeric type of points to convert to.
   * @tparam _InType The numeric type that the points are being converted from.
   * @param[in] input The input vector of points to be converted to another type.
   * @return std::vector<cv::Point3_<_OutType>> The output converted points.
   */
  template<typename _OutType, typename _InType>
  std::vector<cv::Point3_<_OutType>> convertToPoints3(std::vector<cv::Point3_<_InType>>& input) {
    std::vector<cv::Point3_<_OutType>> output(input.size());
    std::transform(input.begin(), input.end(), output.begin(), [](const cv::Point3_<_InType>& p) {
      return cv::Point3_<_OutType>(static_cast<_OutType>(p.x), static_cast<_OutType>(p.y), static_cast<_OutType>(p.z));
    });
    return output;
  }
}