#pragma once

#include <string>
#include <vector>

#include <opencv2/core.hpp>

namespace rv {
  struct Target {
    std::string name;
    std::vector<cv::Point2f> shape;

    void write(cv::FileStorage& fs) const {
      fs << "{" << "Name" << name << "Shape" << shape << "}";
    }

    void read(const cv::FileNode& node) {
      node["Name"] >> name;
      node["Shape"] >> shape;
    }
  };

  static void write(cv::FileStorage& fs, const std::string&, const rv::Target& x) { 
    x.write(fs);
  }
  
  static void read(const cv::FileNode& node, rv::Target& x, const rv::Target& default_value = rv::Target()){
    if(node.empty()) {
      x = default_value;
    } else {
      x.read(node);
    }
  }

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

    void write(cv::FileStorage& fs) const {
      fs << "{" << "Radius" << radius << "Center" << center << "}";
    }

    void read(const cv::FileNode& node) {
      node["Radius"] >> radius;
      node["Center"] >> center;
    }
  };

  static void write(cv::FileStorage& fs, const std::string&, const rv::Ball& x) { 
    x.write(fs);
  }
  
  static void read(const cv::FileNode& node, rv::Ball& x, const rv::Ball& default_value = rv::Ball()){
    if(node.empty()) {
      x = default_value;
    } else {
      x.read(node);
    }
  }

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

  bool approximateNGon(std::vector<cv::Point2f>& src, std::vector<cv::Point2f>& dst, int n, double start = 0, double end = 100, double step = 0.1);

  void reorderPoints(std::vector<cv::Point2f>& points);

  cv::Mat normalizedContourImage(std::vector<cv::Point2f> contour, std::vector<cv::Point2f>& projectedContour, cv::Mat& image);

  std::vector<rv::TargetMatch> matchTargetPoints(std::vector<rv::TargetMatch>& matches);

  std::vector<rv::TargetMatch> findTargets(std::vector<std::vector<cv::Point>> contours, std::vector<rv::Target> targets, double minArea, double minMatch);

  std::vector<rv::TargetPose> estimateTargetPose(std::vector<TargetMatch> matches, cv::Mat cameraMatrix, cv::Mat distortion);

  std::vector<rv::CircleMatch> findCircles(std::vector<std::vector<cv::Point>> contours, double minArea, double minMatch);

  std::vector<rv::BallPose> estimateBallPose(std::vector<CircleMatch> circles, rv::Ball ball, cv::Mat cameraMatrix, cv::Mat distortion);
}