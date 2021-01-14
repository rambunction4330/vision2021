#pragma once

#include <string>
#include <vector>

#include <opencv2/core.hpp>

namespace rv {
  struct Target {
    std::string name;
    std::vector<cv::Point2f> shape;
  };

  struct TargetMatch {
    std::vector<cv::Point2f> shape;
    rv::Target target;
    double match;
  };

  struct TargetPose {
    rv::TargetMatch match;
    cv::Mat tvec;
    cv::Mat rvec;
  };

  struct Ball {
    float radius;
    cv::Point3f center;

    std::vector<cv::Point3f> points();
  };

  struct Circle {
    float radius; 
    cv::Point2f center;

    double area();
    std::vector<cv::Point2f> points();
  };

  struct CircleMatch {
    std::vector<cv::Point> contour;
    rv::Circle circle;
    double match;
  };

  struct BallPose {
    rv::CircleMatch circleMatch;
    rv::Ball ball;
    cv::Mat tvec;
    cv::Mat rvec;
  };

  std::vector<rv::TargetMatch> findTargets(std::vector<std::vector<cv::Point>> contours, std::vector<rv::Target> targets, double minArea, double minMatch);

  std::vector<rv::TargetPose> estimateTargetPose(std::vector<TargetMatch> matches, cv::Mat cameraMatrix, cv::Mat distortion);

  std::vector<rv::CircleMatch> findCircles(std::vector<std::vector<cv::Point>> contours, double minArea, double minMatch);

  std::vector<rv::BallPose> estimateBallPose(std::vector<CircleMatch> circles, rv::Ball ball, cv::Mat cameraMatrix, cv::Mat distortion);
}