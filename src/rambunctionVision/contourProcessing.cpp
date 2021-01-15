#include <rambunctionVision/contourProcessing.hpp>

#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

#include "rambunctionVision/conversions.hpp"

namespace rv {
  std::vector<cv::Point3f> rv::Ball::points() {
    return std::vector<cv::Point3f> {
      center,
      cv::Point3f{radius,  0, 0} + center,
      cv::Point3f{0, radius,  0} + center,
      cv::Point3f{-radius, 0, 0} + center,
      cv::Point3f{0, -radius, 0} + center
    };
  }

   double rv::Circle::area() {
    return radius * radius * M_PI;
  }

  std::vector<cv::Point2f> rv::Circle::points() {
    return std::vector<cv::Point2f> {
      center,
      cv::Point2f{radius,  0} + center,
      cv::Point2f{0,  radius} + center,
      cv::Point2f{-radius, 0} + center,
      cv::Point2f{0, -radius} + center
    };
  }

  bool approximateNGon(std::vector<cv::Point2f>& points, int n, double start, double end, double step) {
    std::vector<cv::Point2f> aprox;
    for (double epsilon = start; epsilon < end; epsilon += step) {
      cv::approxPolyDP(points, aprox, epsilon, true);

      if (aprox.size() < n) {
        return false;
      }

      if (aprox.size() == n) {
        points = aprox;
        return true;
      }
    }

    return false;
  }

  std::vector<rv::TargetMatch> findTargets(std::vector<std::vector<cv::Point>> contours, std::vector<rv::Target> targets, double minArea, double minMatch) {
    std::vector<rv::TargetMatch> matches;
    for (auto& contour : contours) {
      double contourArea = cv::contourArea(contour);
      if (contourArea < minArea) {
        continue;
      }

      rv::Target matchingTarget;
      double bestMatch = minMatch;
      for (auto& target : targets) {
        double matchValue = cv::matchShapes(contour, target.shape, cv::CONTOURS_MATCH_I1, 0);

        if (matchValue < bestMatch) {
            matchingTarget = target;
            bestMatch = matchValue;
        }
      }

      if (!matchingTarget.shape.empty()) {
        matches.push_back(rv::TargetMatch {rv::convertToPoints<float>(contour), matchingTarget, bestMatch});
      }
    }

    return matches;
  }

  std::vector<rv::TargetPose> estimateTargetPose(std::vector<TargetMatch> matches, cv::Mat cameraMatrix, cv::Mat distortion) {
    std::vector<rv::TargetPose> positions;
    for (auto& match : matches) {
      rv::TargetPose position;
      cv::solvePnP(rv::convertToPoints3<float>(match.target.shape), match.shape, cameraMatrix, distortion, position.rvec, position.tvec);
      positions.push_back(position);
    }
    return positions;
  }  

  std::vector<rv::CircleMatch> findCircles(std::vector<std::vector<cv::Point>> contours, double minArea, double minMatch) {
    std::vector<rv::CircleMatch> matches;
    for (auto& contour : contours) {

      double contourArea = cv::contourArea(contour);
      if (contourArea < minArea) {
        continue;
      }

      rv::Circle circle;
      cv::minEnclosingCircle(contour, circle.center, circle.radius);
      double matchValue = contourArea / circle.area();

      if (matchValue > minMatch) {
        matches.push_back({contour, circle, matchValue});
      }
    }
    return matches;
  }

  std::vector<rv::BallPose> estimateBallPose(std::vector<CircleMatch> circles, rv::Ball ball, cv::Mat cameraMatrix, cv::Mat distortion) {
    std::vector<rv::BallPose> positions;
    for (auto& circle : circles) {
      rv::BallPose position;
      cv::solvePnP(ball.points(), circle.circle.points(), cameraMatrix, distortion, position.rvec, position.tvec);
      positions.push_back(position);
    }
    return positions;
  }
}