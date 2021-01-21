#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <filesystem>

#include <opencv2/core.hpp>
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
  "{ h ? help usage |   | prints this message                   }"
  "{ c camera       | 0 | Camera id used for thresholding       }"
  "{ i images       |   | Optional images for HSV Tunning       }"
  "{ b blur         |   | Whether to present a blur slider      }"
  "{ m morph        |   | Whether to present morphology sliders }"
  "{ in input       |   | Input file                            }"
  "{ out output     |   | Output file                           }";

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
  bool useBlurSlider = parser.has("blur");
  bool useMorphSlider = parser.has("morph");
  std::string inputFile = parser.get<std::string>("input");
  std::string outputFile = parser.get<std::string>("output");

  // Cheack for errors
  if (!parser.check()) {
    parser.printErrors();
    return 0;
  }

  cv::FileStorage fileStorage;

  //****************************************************************************
  // GUI setup
  //****************************************************************************

  // Threshold variables
  rv::Threshold threshold;
  if (inputFile != "" && std::filesystem::exists(inputFile)) {
    fileStorage.open(inputFile, cv::FileStorage::READ);
    if (fileStorage.isOpened()) {
      fileStorage["Threshold"] >> threshold;
    }
    fileStorage.release();
  }

  int openSize = 15, openShape = 0, closeSize = 15, closeShape = 0;

  // Create window
  cv::namedWindow("HSV Tunning");

  // Create Trackbars
  cv::createTrackbar("High H",  "HSV Tunning", &threshold.highH(),  180);
  cv::createTrackbar("Low H", "HSV Tunning", &threshold.lowH(), 180);
  cv::createTrackbar("High S",  "HSV Tunning", &threshold.highS(),  255);
  cv::createTrackbar("Low S", "HSV Tunning", &threshold.lowS(), 255);
  cv::createTrackbar("High V",  "HSV Tunning", &threshold.highV(),  255);
  cv::createTrackbar("Low V", "HSV Tunning", &threshold.lowV(), 255);

  if (useBlurSlider) {
    cv::createTrackbar("Blur Size", "HSV Tunning", &threshold.blurSize, 100);
  }

  if (useMorphSlider) {
    cv::createTrackbar("Open Size", "HSV Tunning", &openSize, 100);
    cv::createTrackbar("Open Type", "HSV Tunning", &openShape, 2);
    cv::createTrackbar("Close Size", "HSV Tunning", &closeSize, 100);
    cv::createTrackbar("Close Type", "HSV Tunning", &closeShape, 2);
  }

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
  bool showThresh = true, showBlur = false, useBallDetection = false, useTargetDetection = false;
  cv::Mat image, thresh, display;
  while (true) {
    
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

    if (useMorphSlider) {
      threshold.closeMatrix = cv::getStructuringElement(closeShape, {std::max(closeSize, 1), std::max(closeSize, 1)});
      threshold.openMatrix = cv::getStructuringElement(openShape, {std::max(openSize, 1), std::max(openSize, 1)});
    }

    rv::thresholdImage(image, thresh, threshold);

    // Conditionaly display a thresheld view 
    if (showThresh) {
      cv::cvtColor(thresh, display, cv::COLOR_GRAY2BGR);
    } else if (showBlur) {
      cv::blur(image, display, {std::max(threshold.blurSize, 1), std::max(threshold.blurSize, 1)});
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
    useBallDetection = (key == 'd') ? !useBallDetection : useBallDetection;
    showBlur = (key == 'b') ? !showBlur : showBlur;

    imageIndex = (key == '>' || key == '.') ? std::min(imageIndex + 1, (int) images.size()-1) : imageIndex;
    imageIndex = (key == '<' || key == ',') ? std::max(imageIndex - 1, 0) : imageIndex;

    if (key == 's' && outputFile != "") {
      fileStorage.open(outputFile, cv::FileStorage::WRITE);
      if (fileStorage.isOpened()) {
        fileStorage << "Threshold" << threshold;
      }
      fileStorage.release();
      break;
    }

    if (key == 'q' || key == 27) {
      break;
    }
  }
  // Cleanup when done.
  cv::destroyAllWindows();
  capture.release();
  cv::waitKey(1);
  fileStorage.release();
  return 0;
}