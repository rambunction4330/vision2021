/**
 * @file imageprocessing.hpp
 * @author George Jurgiel (gcjurgiel@icloud.com)
 * @brief Functions and structers to proccess images.
 * @version 0.1
 * @date 2021-01-23
 * 
 * @copyright Copyright (c) 2021
 */

#pragma once

#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

/**
 * @brief 'Rambunction Vision' namespace to store shared code.
 */
namespace rv {

  /**
   * @brief Structer holding data to threshold an image.
   * 
   * Holds the high and low values for thresholding in the hsv color space, as
   * well as varables to determin the blur size and morphology matrices to
   * remove noise and close holes.
   * 
   * @see thresholdImage
   */
  struct Threshold {
    cv::Scalar_<int> high = {180, 255, 255}; /**< The upper bound of the hsv threshold. */
    cv::Scalar_<int> low  = {  0,   0,   0}; /**< The lower bound of the hsv threshold. */

    int blurSize = 15; /**< The size of the square blur filter to remove image noise. */

    cv::Mat openMatrix  = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15,15)); /**< The kernal to remove noise in thresholding. */
    cv::Mat closeMatrix = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15,15)); /**< The kernal to close holes in thresholding. */

    int& highH() { return high[0]; } /**< Gets a refrece to the hue part of the high scalar. */
    int& lowH()  { return low[0];  } /**< Gets a refrece to the hue part of the low scalar. */
    int& highS() { return high[1]; } /**< Gets a refrece to the saturation part of the high scalar. */
    int& lowS()  { return low[1];  } /**< Gets a refrece to the saturation part of the low scalar. */
    int& highV() { return high[2]; } /**< Gets a refrece to the value part of the high scalar. */
    int& lowV()  { return low[2];  } /**< Gets a refrece to the value part of the low scalar. */

    void setHighH(int value) { highH() = std::clamp(value, lowH() + 1, 180); } /**< Sets the hue part of the high scalar with bound checks. */
    void setLowH(int value)  { lowH()  = std::clamp(value,  0, highH() - 1); } /**< Sets the hue part of the low scalar with bound checks. */
    void setHighS(int value) { highV() = std::clamp(value, lowS() + 1, 255); } /**< Sets the saturation part of the high scalar with bound checks. */
    void setLowS(int value)  { lowV()  = std::clamp(value,  0, highS() - 1); } /**< Sets the saturation part of the low scalar with bound checks. */
    void setHighV(int value) { highV() = std::clamp(value, lowV() + 1, 255); } /**< Sets the value part of the high scalar with bound checks. */
    void setLowV(int value)  { lowV()  = std::clamp(value,  0, highV() - 1); } /**< Sets the value part of the low scalar with bound checks. */

    void write(cv::FileStorage& fs) const {
      fs << "{" << "High" << high << "Low" << low << "BlurSize" << blurSize << "OpenMatrix" << openMatrix << "CloseMatrix" << closeMatrix << "}";
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
  
  static void read(const cv::FileNode& node, rv::Threshold& x, const rv::Threshold& default_value = rv::Threshold()) {
    if(node.empty()) {
      x = default_value;
    } else {
      x.read(node);
    }
  }

  /**
   * @brief Thresholds an image in the HSV color space.
   * 
   * The function fist blurs the image, then the image is then converted into
   * the HSV color space to be thresheld. After thresholding, two 
   * morphalogical operations are applied to close any holes and remove noise.
   * 
   * @param[in] src The input image.
   * @param[out] dst The output thresheld image.
   * @param[in] threshold The parameters to threshold the image by.
   * 
   * @see Threshold
   */
  void thresholdImage(cv::Mat& src, cv::Mat& dst, rv::Threshold threshold);

  /**
   * @brief Extracs all the image files from a given directory
   * 
   * Iterates over a directory and tries to extract each file as an image. 
   * All files that can be read by OpenCV are added to the vector, while all 
   * otehrs are skiped. The iteration is not recursive.
   * 
   * @param[in] filepath The filepath to the directory with the images.
   * @param[out] images  The output vector of images in the directory.
   * @return false, if the filepath can't be found.
   */
  bool extractImagesFromDirectory(std::string filepath, std::vector<cv::Mat>& images);
}