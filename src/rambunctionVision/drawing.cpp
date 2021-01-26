#include "rambunctionVision/drawing.hpp"

#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>

namespace rv {
    void drawAxis(cv::Mat& image, double size, cv::Mat cameraMatrix, cv::Mat distortion, cv::Mat rvec, cv::Mat tvec) {
        std::vector<cv::Point3f> objectPoints = {
            cv::Point3f(0,0,0),
            cv::Point3f(1,0,0),
            cv::Point3f(0,1,0),
            cv::Point3f(0,0,1),
        };

        std::vector<cv::Point2f> imagePoints;
        cv::projectPoints(objectPoints, rvec, tvec, cameraMatrix, distortion, imagePoints);

        cv::line(image, imagePoints[0], imagePoints[1], {255,0,0}, 4);
        cv::line(image, imagePoints[0], imagePoints[2], {0,255,0}, 4);
        cv::line(image, imagePoints[0], imagePoints[3], {0,0,255}, 4);
    } 
}