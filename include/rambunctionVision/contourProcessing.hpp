/**
 * @file contourProcessing.hpp
 * @author George Jurgiel (gcjurgiel@icloud.com)
 * @brief Functions and structs to proccess contours.
 * @version 0.1
 * @date 2021-01-23
 * 
 * @copyright Copyright (c) 2021
 */

#pragma once

#include <string>
#include <vector>

#include <opencv2/core.hpp>

/**
 * @brief 'Rambunction Vision' namespace to store shared code.
 */
namespace rv {

  /**
   * @brief A target shape and name.
   * 
   * @see fTargetMatch TargetPose findTargets
   */
  struct Target {
    std::string name; /**< The name of the target. */
    std::vector<cv::Point2f> shape; /**< The shape of the target. */

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

  /**
   * @brief A contour and it's matching target.
   * 
   * @see Target findTargets matchTargetPoints estimateTargetPose
   */
  struct TargetMatch {
    std::vector<cv::Point2f> shape; /**< The contour. */
    rv::Target target; /**< The best matching target. */
    double match; /**< How good the match is (0.0 - 1.0). */
  };

  /**
   * @brief The 3d position of a target
   * 
   * @see TargetMatch Target estimateTargetPose
   */
  struct TargetPose {
    rv::TargetMatch match; /**< The matching target and contour. */
    cv::Mat tvec; /**< The translation (position) of the target. */
    cv::Mat rvec; /**< The rotation of the target. */
  };

  /**
   * @brief A ball with a given radius and center.
   * 
   * @see Circle
   */
  struct Ball {
    float radius; /**< The radius of the ball. */
    cv::Point3f center; /**< The location of the center of the ball. */

    std::vector<cv::Point3f> points(); /**< The center followed by the extreme points of the ball. */

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

  /**
   * @brief A circle with a given radius and center.
   * 
   * @see Ball findCircles
   */
  struct Circle {
    float radius; /**< The circle radius. */
    cv::Point2f center; /**< The location of the center of the circle. */

    double area(); /**< The area of the circle (pi*r^2). */
    std::vector<cv::Point2f> points(); /**< The center followed by the extreme points of the circle. */
  };

  /**
   * @brief A contour and it's closest matching circle.
   * 
   * @see Circle findCircles estimateBallPose
   */
  struct CircleMatch {
    std::vector<cv::Point> contour; /**< The conour. */
    rv::Circle circle; /**< The circle that best matches the contour. */
    double match; /**< How good the match is (0.0 - 1.0). */
  };

  /**
   * @brief The estimated position of a ball.
   * 
   * @see CircleMatch Ball Circle estimateBallPose
   */
  struct BallPose {
    rv::CircleMatch circleMatch; /**< The matchin circle. */
    rv::Ball ball; /**< The corosponging ball. */
    cv::Mat tvec; /**< The translation (position) of the ball. */
    cv::Mat rvec; /**< The rotation of the ball. */
  };

  /**
   * @brief Approximates a contour to have n sides.
   * 
   * The function iterativly attempts aproximations with a higher error
   * untill one is found with n sides.
   * 
   * @param[in] src The input points.
   * @param[out] dst The output Points.
   * @param[in] n The target number of sides.
   * @param[in] start The guess to start with.
   * @param[in] end The guess to end with.
   * @param[in] step The change in error between each iteration.
   * @return true, if a correct approimation is found.
   * @return false, if no such approximation can be found.
   * 
   * @see matchTargetPoints
   */
  bool approximateNGon(std::vector<cv::Point2f>& src, std::vector<cv::Point2f>& dst, int n, double start = 0, double end = 100, double step = 0.1);

  /**
   * @brief Reorders a vector of points to start with the one closest to the origin.
   * 
   * The vector is rotated so the point closest to the origin is first. If
   * the points are in a counter-clockwise order, they will be corrected to
   * a clockwise order.
   * 
   * @param[in,out] points The point to be reorder.
   * 
   * @see matchTargetPoints
   */
  void reorderPoints(std::vector<cv::Point2f>& points);

  /**
   * @brief Normalizes a contour into a 255x255 binary image.
   * 
   * A rotated bounding box of the contour is found, and the points are 
   * transformed such that the box becomes the image frame. The contour is 
   * then drawn onto the image filled. This can then be used for an improved 
   * shape match value and orientation.
   * 
   * @param[in] contour The input contour to be normalized.
   * @param[out] projectedContour The output points of the contour after being transformed.
   * @param[out] image The ouput binary image of the transformed points.
   * @return cv::Mat The matrix used to transform the points into the image.
   * 
   * @see matchTargetPoints
   */
  cv::Mat normalizedContourImage(std::vector<cv::Point2f> contour, std::vector<cv::Point2f>& projectedContour, cv::Mat& image);

  /**
   * @brief Processes a match so the contour an target points aline.
   * 
   * Both the target and contour are transformed to fit in a 255x255 square.
   * They are then drawn and their overlap is calculated at each orintation so
   * they will have matching orientation. The contour points are simplifyed to
   * have the same number of sides as the target, and the points of each are 
   * reordered to assure corospondence becofre being transformed back to thier
   * origional position.
   * 
   * @param[in] matches The input vector of matches.
   * @return std::vector<rv::TargetMatch> the ouput processed matches.
   * 
   * @see TargetMatch normalizedContourImage approximateNGon reorderPoints
   */
  std::vector<rv::TargetMatch> matchTargetPoints(std::vector<rv::TargetMatch>& matches);

  /**
   * @brief Finds the targets that best matches each contour.
   * 
   * @param[in] contours The input contours to be matched.
   * @param[in] targets The targets to match the contours against.
   * @param[in] minArea The minimum contour area allowable.
   * @param[in] maxMatch The mamatch value allowable (lower is better).
   * @return std::vector<rv::TargetMatch> The paired up contours and targets.
   * 
   * @see Target TargetMatch
   */
  std::vector<rv::TargetMatch> findTargets(std::vector<std::vector<cv::Point>> contours, std::vector<rv::Target> targets, double minArea, double maxMatch);

  /**
   * @brief Estimates the target position using a solvePnP.
   * 
   * @param[in] matches Input matches to solve the position for.
   * @param[in] cameraMatrix The intrnsic camera matrix for 2d-3d corospondence.
   * @param[in] distortion The coefficents to acount for lense distortion.
   * @return std::vector<rv::TargetPose> The matches along with thier solved position.
   * 
   * @see TargetMatch TargetPose
   */
  std::vector<rv::TargetPose> estimateTargetPose(std::vector<TargetMatch> matches, cv::Mat cameraMatrix, cv::Mat distortion);

  /**
   * @brief Find the closest matchng circle of a contour.
   * 
   * @param[in] contours The input contours to be matched with circles.
   * @param[in] minArea The minimum allowable contour area.
   * @param[in] minMatch The minimum allowable match value (0.0-1.0).
   * @return std::vector<rv::CircleMatch> The output contours matched with thier closest matching circle.
   * 
   * @see Circle CircleMatch estimateBallPose Ball BallPose
   */
  std::vector<rv::CircleMatch> findCircles(std::vector<std::vector<cv::Point>> contours, double minArea, double minMatch);

  /**
   * @brief Estimates the position of a ball
   * 
   * @param[in] circles The circles to find the position of.
   * @param[in] ball The size of the balls to find the position of.
   * @param[in] cameraMatrix The intrnsic camera matrix for 2d-3d corospondence.
   * @param[in] distortion The coefficents to acount for lense distortion.
   * @return std::vector<rv::BallPose> The output positions of the balls
   * 
   * @see CircleMatch BallPose findCircles Cirlce Ball
   */
  std::vector<rv::BallPose> estimateBallPose(std::vector<CircleMatch> circles, rv::Ball ball, cv::Mat cameraMatrix, cv::Mat distortion);
}