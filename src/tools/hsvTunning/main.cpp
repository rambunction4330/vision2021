#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <filesystem>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "rambunctionVision/contourProcessing.hpp"
#include "rambunctionVision/imageProcessing.hpp"

int main(int argc, char** argv) {

  //****************************************************************************
  // Argument Parsing
  //****************************************************************************
  
  // Keys for argument parsing
  const std::string keys = 
  "{ h ? help usage |   | prints this message             }"
  "{ c camera       | 0 | Camera id used for thresholding }"
  "{ i images       |   | Optional images for HSV Tunning }";

  // Parser object
  cv::CommandLineParser parser(argc, argv, keys);
  parser.about("\nvision2021 v0.0.0 hsvTunning"
               "\nTool to find hsv thresholding value\n");

  // Show help if help is flagged.
  if (parser.has("help")) {
    parser.printMessage();
    return 0;
  }

  // Get arguments from the parser
  int cameraID = parser.get<double>("camera");
  bool useImages = parser.has("images");
  std::string pathToImages = parser.get<std::string>("images");

  // Cheack for errors
  if (!parser.check()) {
    parser.printErrors();
    return 0;
  }

  //****************************************************************************
  // GUI setup
  //****************************************************************************

  // Threshold variables
  rv::Threshold threshold;

  // Create window
  cv::namedWindow("HSV Tunning");

  // Create Trackbars
  cv::createTrackbar("High H",  "HSV Tunning", &threshold.highH(),  180);
  cv::createTrackbar("Low H", "HSV Tunning", &threshold.lowH(), 180);
  cv::createTrackbar("High S",  "HSV Tunning", &threshold.highS(),  255);
  cv::createTrackbar("Low S", "HSV Tunning", &threshold.lowS(), 255);
  cv::createTrackbar("High V",  "HSV Tunning", &threshold.highV(),  255);
  cv::createTrackbar("Low V", "HSV Tunning", &threshold.lowV(), 255);

  //****************************************************************************
  // Camera Initialization
  //****************************************************************************

  // Camera for video tunning
  cv::VideoCapture capture;

  // Image vector for image tunning
  std::vector<cv::Mat> images;

  if (useImages) {
    if (!rv::extractImagesFromDirectory(pathToImages, images)) {
      std::cerr << "File: '" << pathToImages << "' does not exist\n";
      return 0;
    }

    if (images.empty()) {
      std::cerr << "No images could be found at" << pathToImages << "\n";
      return 0;
    }

  } else {
    // Camera
    bool opened = capture.open(cameraID);

    // Check camera data
    if (!opened) {
      std::cerr << "Could access camera\n";
      return 0;
    }
  }

  int imageIndex = 0;
  bool showThresh = true, useBallDetection = false, useTargetDetection = false;
  cv::Mat image, thresh, display;
  while (true) {
    imageIndex = std::clamp(imageIndex, 0, (int) images.size()-1);
    
    if (useImages) {
      image = images[imageIndex];
    } else {
      // Get the next frame from the camera
      capture >> image;

      // Check camera data
      if (image.empty()) {
        std::cerr << "Lost connection to camera\n";
        break;
      }
    }

    //**************************************************************************
    // Threshold Image
    //**************************************************************************

    rv::thresholdImage(image, thresh, 15, threshold, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(15, 15)), cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(15, 15)));

    // Conditionaly display a thresheld view 
    if (showThresh) {
      cv::cvtColor(thresh, display, cv::COLOR_GRAY2BGR);
    } else {
      image.copyTo(display);
    }

    //**************************************************************************
    // Ball Detection
    //**************************************************************************

    if (useBallDetection) {
      // Find contours in the image
      std::vector<std::vector<cv::Point>> contours;
      cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

      std::vector<rv::CircleMatch> circles = rv::findCircles(contours, 50, 0.60);

      for (auto& circle : circles) {
        cv::circle(display, circle.circle.center, (int) circle.circle.radius, {0,0,255}, 3);
      }
    }

    // Show the frames.
    cv::imshow("HSV Tunning", display);

    // Parse key presses.
    int key = cv::waitKey(30);

    showThresh = (key == 't') ? !showThresh : showThresh;
    useBallDetection = (key == 'b') ? !useBallDetection : useBallDetection;
    useTargetDetection = (key == 'c') ? !useTargetDetection : useTargetDetection;

    imageIndex += (key == '>' || key == '.') ? 1 : 0;
    imageIndex -= (key == '<' || key == ',') ? 1 : 0;

    if (key == 'q' || key == 27) {
      break;
    }
  }
  // Cleanup when done.
  cv::destroyAllWindows();
  capture.release();
  cv::waitKey(1);
  return 0;
}