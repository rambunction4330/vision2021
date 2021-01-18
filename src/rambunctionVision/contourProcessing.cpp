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

  bool approximateNGon(std::vector<cv::Point2f>& src, std::vector<cv::Point2f>& dst, int n, double start, double end, double step) {
    std::vector<cv::Point2f> aprox;
    for (double epsilon = start; epsilon < end; epsilon += step) {
      cv::approxPolyDP(src, aprox, epsilon, true);

      if (aprox.size() < n) {
        return false;
      }

      if (aprox.size() == n) {
        dst = aprox;
        return true;
      }
    }

    return false;
  }

  void reorderPoints(std::vector<cv::Point2f>& points) {
    auto start = std::min_element(points.begin(), points.end(), [](const cv::Point2f& a, const cv::Point2f& b) {
      return std::sqrt((a.x * a.x) + (a.y * a.y)) < std::sqrt((b.x * b.x) + (b.y * b.y));
    });

    std::rotate(points.begin(), start, points.end());
  }

  cv::Mat normalizedContourImage(std::vector<cv::Point2f> contour, std::vector<cv::Point2f>& projectedContour, cv::Mat& image) {
    cv::RotatedRect rect = cv::minAreaRect(contour);
    cv::Point2f srcPoints[4];
    rect.points(srcPoints);

    const static cv::Point2f dstPoints[4] = {
      {0, 255},
      {0,0},
      {255, 0},
      {255, 255}
    };

    cv::Mat transform = cv::getPerspectiveTransform(srcPoints, dstPoints);
    cv::transform(contour, projectedContour, transform);

    image = cv::Mat(255, 255, CV_8UC1, 0);
    cv::drawContours(image, rv::inVector(rv::convertToPoints<int>(projectedContour)), 0, 255, -1);

    return transform;
  }

  std::vector<rv::TargetMatch> matchTargetPoints(std::vector<rv::TargetMatch>& matches) {
    std::vector<rv::TargetMatch> output;
    for (auto& match : matches) {
      cv::Mat shapeImage, targetImage, compareImage;
      std::vector<cv::Point2f> projectedShape, projectedTarget;
      cv::Mat shapeTransform = normalizedContourImage(match.shape, projectedShape, shapeImage);
      cv::Mat targetTransform = normalizedContourImage(match.target.shape, projectedTarget, targetImage);

      cv::bitwise_xor(shapeImage, targetImage, compareImage);
      double bestValue = cv::countNonZero(compareImage);
      int bestRot = 0;

      for (int rotation = 0; rotation < 3; rotation++) {
        cv::Mat rotImage;
        cv::rotate(shapeImage, rotImage, rotation);

        cv::bitwise_xor(rotImage, targetImage, compareImage);
        double value = cv::countNonZero(compareImage);

        if (value < bestValue) {
          bestValue = value;
          bestRot = (rotation * 90) + 90;
        }
      }

      cv::Mat rotation;
      cv::vconcat(cv::getRotationMatrix2D({127, 127}, bestRot, 1), cv::Mat(1, 3, CV_32FC1, {0,0,1}), rotation);
      cv::transform(projectedShape, projectedShape, rotation);

      approximateNGon(projectedShape, projectedShape, match.target.shape.size(), 15, 50, 0.5);

      reorderPoints(projectedShape);
      reorderPoints(projectedTarget);


      std::vector<cv::Point2f> outputContour, outputTarget;
      cv::transform(projectedShape, outputContour, (shapeTransform * rotation).inv());
      cv::transform(projectedTarget, outputTarget, targetTransform.inv());

      rv::TargetMatch outputMatch = match;
      outputMatch.shape = outputContour;
      outputMatch.target.shape = outputTarget;
      outputMatch.match = 1 - (bestRot / (255 * 255));

      output.push_back(outputMatch);
    }

    return output;
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