/**
 * @file drawing.hpp
 * @author George Jurgiel (gcjurgiel@icloud.com)
 * @brief Functions for drawing overlays on images
 * @version 0.1
 * @date 2021-01-26
 * 
 * @copyright Copyright (c) 2021
 */
#pragma once

#include <opencv2/core.hpp>

/**
 * @brief 'Rambunction Vision' namespace to store shared code.
 */
namespace rv {
    /**
     * @brief Draws a 3d onto an image given rotation and translation vectors from position estimation.
     * 
     * @param image The image to draw the axis on.
     * @param size The size of the axis in the object space.
     * @param cameraMatrix The Matrix representing the intresic camera properties.
     * @param distortion The cooefficents represnting camera lense distortion.
     * @param rvec The vector representing object rotation.
     * @param tvec The vector representing object translation.
     * 
     * @see estimateBallPose estimateTargetPose
     */
    void drawAxis(cv::Mat& image, double size, cv::Mat cameraMatrix, cv::Mat distortion, cv::Mat rvec, cv::Mat tvec);
}