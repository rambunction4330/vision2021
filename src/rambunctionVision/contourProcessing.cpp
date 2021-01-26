#include <rambunctionVision/contourProcessing.hpp>

#include <vector>
#include <iostream>

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

    // Continually try aproximations with a lower 
    // accuracy untill it has n sides.
    for (double epsilon = start; epsilon < end; epsilon += step) {
      cv::approxPolyDP(src, aprox, epsilon, true);

      // If the approximation has too few sides 
      // it failed, so return flase.
      if (aprox.size() < n) {
        return false;
      }

      // If the aproximation  has the corrct number of sides 
      // return true, and set the output to the aproximation.
      if (aprox.size() == n) {
        dst = aprox;
        return true;
      }
    }
    // If it gets to the end and no proper aproximation 
    // has been found, retun false.
    return false;
  }

  void reorderPoints(std::vector<cv::Point2f>& points) {

    // If the points are counter-clockwise, they will be reversed.
    if (cv::contourArea(points, true) < 0) {
      std::reverse(points.begin(), points.end());
    }

    // Find the point that is closes to (0,0)
    auto start = std::min_element(points.begin(), points.end(), [](const cv::Point2f& a, const cv::Point2f& b) {
      return std::sqrt((a.x * a.x) + (a.y * a.y)) < std::sqrt((b.x * b.x) + (b.y * b.y));
    });

    // Rotate the vector so that the closest point is first
    std::rotate(points.begin(), start, points.end());
  }

  cv::Mat normalizedContourImage(std::vector<cv::Point2f> contour, std::vector<cv::Point2f>& projectedContour, cv::Mat& image) {
    // FInd the bounding rotated rect
    cv::RotatedRect rect = cv::minAreaRect(contour);
    cv::Point2f srcPoints[4];
    rect.points(srcPoints);

    // Corners of a 255x255 image
    const static cv::Point2f dstPoints[4] = {
      {0, 255},
      {0,0},
      {255, 0},
      {255, 255}
    };

    // Transform the poins such that the bounding rect
    // is now the fram of 255 x 255 image.
    cv::Mat transform = cv::getPerspectiveTransform(srcPoints, dstPoints);

    // TODO: Fix error here
    cv::perspectiveTransform(contour, projectedContour, transform);

    // Draw the points on to the image
    image = cv::Mat(256, 256, CV_8UC1, cv::Scalar(0));
    cv::drawContours(image, std::vector<std::vector<cv::Point>>(1, rv::convertToPoints<int>(projectedContour)), 0, 255, -1);

    // Return the matrix used to transform the points.
    return transform;
  }

  std::vector<rv::TargetMatch> matchTargetPoints(std::vector<rv::TargetMatch>& matches) {
    std::vector<rv::TargetMatch> output;
    for (auto& match : matches) {
      cv::Mat shapeImage, targetImage, compareImage;
      std::vector<cv::Point2f> projectedShape, projectedTarget;

      // Transforms both the target and contour into a 255x255 image
      cv::Mat shapeTransform = normalizedContourImage(match.shape, projectedShape, shapeImage);
      cv::Mat targetTransform = normalizedContourImage(match.target.shape, projectedTarget, targetImage);

      // Determin which orintatiion of images has the most overlap
      // and thus is the proper orientation of the contour.
      cv::bitwise_xor(shapeImage, targetImage, compareImage);
      double bestValue = cv::countNonZero(compareImage);
      int bestRot = 0;

      for (int rot = 0; rot < 3; rot++) {
        cv::Mat rotImage;
        cv::rotate(shapeImage, rotImage, rot);

        cv::bitwise_xor(rotImage, targetImage, compareImage);
        double value = cv::countNonZero(compareImage);

        if (value < bestValue) {
          bestValue = value;
          bestRot = (rot + 1) * -90;
        }
      }

      // Calculate the rotation matrix of that rotation.
      // An extra row must be added to the bottom to 
      // make it a perspective and not afline transform.
      cv::Mat rotation;
      cv::vconcat(cv::getRotationMatrix2D({128, 128}, bestRot, 1), cv::Matx13d{0, 0, 1}, rotation);

      cv::perspectiveTransform(projectedShape, projectedShape, rotation);

      // Aproximate the contour to the same number of sides as the target
      bool aproximated = approximateNGon(projectedShape, projectedShape, match.target.shape.size(), 15, 50, 0.5);

      if (!aproximated) {
        continue;
      }

      // Reorder both the contour and the target so that they
      // start with the point closes to the origin. This assures
      // Point corospondence between the two.
      reorderPoints(projectedShape);
      reorderPoints(projectedTarget);

      // Transform all the points back to thier origional locations
      std::vector<cv::Point2f> outputContour, outputTarget;
      cv::perspectiveTransform(projectedShape, outputContour, (rotation * shapeTransform).inv());
      cv::perspectiveTransform(projectedTarget, outputTarget, targetTransform.inv());

      // Add the modifies contour and target to the match
      rv::TargetMatch outputMatch = match;
      outputMatch.shape = outputContour;
      outputMatch.target.shape = outputTarget;
      // Update the match value with a more accurate one
      // creaated during orientation matching.
      outputMatch.match = 1 - (bestRot / (255 * 255));

      output.push_back(outputMatch);
    }
    return output;
  }

  std::vector<rv::TargetMatch> findTargets(std::vector<std::vector<cv::Point>> contours, std::vector<rv::Target> targets, double minArea, double minMatch) {
    std::vector<rv::TargetMatch> matches;

    for (auto& contour : contours) {

      // Make sure the contoyr isn't too small.
      double contourArea = cv::contourArea(contour);
      if (contourArea < minArea) {
        continue;
      }

      // Find the target that best matches each contour.
      rv::Target matchingTarget;
      double bestMatch = minMatch;
      for (auto& target : targets) {
        double matchValue = cv::matchShapes(contour, target.shape, cv::CONTOURS_MATCH_I1, 0);

        if (matchValue < bestMatch) {
            matchingTarget = target;
            bestMatch = matchValue;
        }
      }

      // If an adequet matching target could be found, add in to the vector. 
      if (!matchingTarget.shape.empty()) {
        matches.push_back({rv::convertToPoints<float>(contour), matchingTarget, bestMatch});
      }
    }
    return matches;
  }

  std::vector<rv::TargetPose> estimateTargetPose(std::vector<TargetMatch> matches, cv::Mat cameraMatrix, cv::Mat distortion) {
    // Run a position estimation over all the matches.
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

      // Cheak if the contour is too small to consider
      double contourArea = cv::contourArea(contour);
      if (contourArea < minArea) {
        continue;
      }

      rv::Circle circle;
      cv::minEnclosingCircle(contour, circle.center, circle.radius);

      // How much it fills the bounding circle
      // This can also be though of as how circular it is
      double matchValue = contourArea / circle.area();

      // If the match is sufficent, 
      if (matchValue > minMatch) {
        matches.push_back({contour, circle, matchValue});
      }
    }
    return matches;
  }

  std::vector<rv::BallPose> estimateBallPose(std::vector<CircleMatch> circles, rv::Ball ball, cv::Mat cameraMatrix, cv::Mat distortion) {
    std::vector<rv::BallPose> positions;
    // Run a position estimation over all the balls.
    for (auto& circle : circles) {
      rv::BallPose position;
      cv::solvePnP(ball.points(), circle.circle.points(), cameraMatrix, distortion, position.rvec, position.tvec);
      positions.push_back(position);
    }
    return positions;
  }
}