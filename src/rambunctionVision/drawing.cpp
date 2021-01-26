#include "rambunctionVision/drawing.hpp"

#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>

namespace rv {
  void drawAxis(cv::Mat& image, double size, cv::Mat cameraMatrix, cv::Mat distortion, cv::Mat rvec, cv::Mat tvec) {
    std::vector<cv::Point3f> objectPoints = {
      cv::Point3f(0,0,0),
      cv::Point3f(size,0,0),
      cv::Point3f(0,size,0),
      cv::Point3f(0,0,size),
    };

    std::vector<cv::Point2f> imagePoints;
    cv::projectPoints(objectPoints, rvec, tvec, cameraMatrix, distortion, imagePoints);

    cv::line(image, imagePoints[0], imagePoints[1], {255,0,0}, 4);
    cv::line(image, imagePoints[0], imagePoints[2], {0,255,0}, 4);
    cv::line(image, imagePoints[0], imagePoints[3], {0,0,255}, 4);
    } 

  void drawBox(cv::Mat& image, cv::Vec3d size, cv::Point3f corner, cv::Mat cameraMatrix, cv::Mat distortion, cv::Mat rvec, cv::Mat tvec) {
    std::vector<cv::Point3f> objectPoints = {
      cv::Point3f(corner.x, corner.y, corner.z),
      cv::Point3f(size[0] + corner.x, corner.y, corner.z),
      cv::Point3f(size[0] + corner.x, size[1] + corner.y, corner.z),
      cv::Point3f(corner.x, size[1] + corner.y, corner.z),
      cv::Point3f(corner.x, corner.y, size[2] + corner.z),
      cv::Point3f(size[0] + corner.x, corner.y, size[2] + corner.z),
      cv::Point3f(size[0] + corner.x, size[1] + corner.y, size[2] + corner.z),
      cv::Point3f(corner.x, size[1] + corner.y, size[2] + corner.z)
    };

    std::vector<cv::Point2f> imagePoints;
    cv::projectPoints(objectPoints, rvec, tvec, cameraMatrix, distortion, imagePoints);

    cv::line(image, imagePoints[0], imagePoints[1], {0,255,0}, 4);
    cv::line(image, imagePoints[1], imagePoints[2], {0,255,0}, 4);
    cv::line(image, imagePoints[2], imagePoints[3], {0,255,0}, 4);
    cv::line(image, imagePoints[3], imagePoints[0], {0,255,0}, 4);

    cv::line(image, imagePoints[4], imagePoints[5], {0,255,0}, 4);
    cv::line(image, imagePoints[5], imagePoints[6], {0,255,0}, 4);
    cv::line(image, imagePoints[6], imagePoints[7], {0,255,0}, 4);
    cv::line(image, imagePoints[7], imagePoints[4], {0,255,0}, 4);

    cv::line(image, imagePoints[0], imagePoints[4], {0,255,0}, 4);
    cv::line(image, imagePoints[1], imagePoints[5], {0,255,0}, 4);
    cv::line(image, imagePoints[2], imagePoints[6], {0,255,0}, 4);
    cv::line(image, imagePoints[3], imagePoints[7], {0,255,0}, 4);
  }
}