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
  
  // Keys for argument parsing (The flags you can set on the executable)
  const std::string keys = 
  "{ h ? help usage |   | prints this message                   }"
  "{ c camera       | 0 | Camera id used for thresholding       }"
  "{ i images       |   | Optional images for HSV Tunning       }"
  "{ b blur         |   | Whether to present a blur slider      }"
  "{ m morph        |   | Whether to present morphology sliders }"
  "{ in input       |   | Input file                            }"
  "{ out output     |   | Output file                           }";

  // Object to parse any argument given
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

  //****************************************************************************
  // Extract Data From Input Files
  //****************************************************************************

  // -----------------------------
  // Extract input threshold data 
  // -----------------------------

  // Variable to hold any thresholding data
  rv::Threshold threshold;

  // If a file was given, extract the data from that file
  if (inputFile == "") {
    if (std::filesystem::exists(inputFile)) {
      cv::FileStorage storage(inputFile, cv::FileStorage::READ);
      if (!storage.isOpened()) {
        storage["Threshold"] >> threshold;
      } else {
        std::cerr << "Error opening input file: '" << inputFile << "'\n";
        return 0;
      }
      storage.release();
    } else {
      std::cerr << "Could not find input file: '" << inputFile << "'\n";
      return 0;
    }
  }

  // -------------------------
  // Extract input image data 
  // -------------------------

  // Camera for video tunning
  cv::VideoCapture capture;

  // Image vector for image tunning
  std::vector<cv::Mat> images;

  // Setup images if images are given, otherwise setup for camera
  if (pathToImages != "") {
    // Pull images from the folder into the vector of images
    bool foundDir = rv::extractImagesFromDirectory(pathToImages, images);

    // Return any nessesary errors
    if (!foundDir) {
      std::cerr << "File: '" << pathToImages << "' does not exist\n";
      return 0;
    }

    if (images.empty()) {
      std::cerr << "No images could be found at" << pathToImages << "\n";
      return 0;
    }

  } else {
    // Open camera with the given id
    capture.open(cameraID);

    // Check camera data
    if (!capture.isOpened()) {
      std::cerr << "Could access camera with id: '" << cameraID << "'\n";
      return 0;
    }
  }

  //****************************************************************************
  // GUI Setup
  //****************************************************************************

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

  // Conditionaly add extra sliders depending on argument flags

  if (useBlurSlider) {
    cv::createTrackbar("Blur Size", "HSV Tunning", &threshold.blurSize, 100);
  }

  if (useMorphSlider) {
    cv::createTrackbar("Open Size", "HSV Tunning", &openSize, 100);
    cv::createTrackbar("Open Type", "HSV Tunning", &openShape, 2);
    cv::createTrackbar("Close Size", "HSV Tunning", &closeSize, 100);
    cv::createTrackbar("Close Type", "HSV Tunning", &closeShape, 2);
  }

  //**************************************************************************
  // Main Loop
  //**************************************************************************

  // Index of the current image to be shown, if images are being used.
  int imageIndex = 0;

  // Conditionals to display diffrent types of data.
  bool showThresh = true, showBlur = false;
  bool ballDetection = false, targetDetection = false;

  cv::Mat image, thresh, display;
  while (true) {
    // -----------
    // Load image
    // -----------

    // Load up the proper image from either the vector or camera.
    if (pathToImages != "") {
      image = images[imageIndex];
    } else {
      // Get the next frame from the camera.
      capture >> image;

      // Check camera data.
      if (image.empty()) {
        std::cerr << "Lost connection to camera\n";
        break;
      }
    }

    // ---------------
    // Proccess image
    // ---------------
    
    // Pull in data from morph sliders.
    // This doesn't have to be done for the others since they are directly modify the value. 
    if (useMorphSlider) {
      threshold.closeMatrix = cv::getStructuringElement(closeShape, {std::max(closeSize, 1), std::max(closeSize, 1)});
      threshold.openMatrix = cv::getStructuringElement(openShape, {std::max(openSize, 1), std::max(openSize, 1)});
    }

    // Threshold thge image acording to the sliders into the `thresh` variable.
    rv::thresholdImage(image, thresh, threshold);

    // --------------------
    // Prepare for display
    // --------------------

    // Conditionaly display a thresheld, blured, or normal view.
    // These actions each output to a `display` variable to be displayed.
    if (showThresh) {
      // The image is converted back to BGR for overlays.
      cv::cvtColor(thresh, display, cv::COLOR_GRAY2BGR);
    } else if (showBlur) {
      cv::blur(image, display, {std::max(threshold.blurSize, 1), std::max(threshold.blurSize, 1)});
    } else {
      image.copyTo(display);
    }

    // Find contours in the image for ball and image detection
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

    // Conditionaly attempt to finds and show balls in the image.
    if (ballDetection) {
      // Find all the contours that are sufficently circular
      std::vector<rv::CircleMatch> circles = rv::findCircles(contours, 50, 0.60);

      // Draw all the circles
      for (auto& circle : circles) {
        cv::circle(display, circle.circle.center, static_cast<int>(circle.circle.radius), {0,0,255}, 3);
      }
    }

    // Show the frames.
    cv::imshow("HSV Tunning", display);

    // -----------------
    // Pasre Keystrokes
    // -----------------

    // key that was pressed
    int key = cv::waitKey(30);

    // Toggle display variables
    showThresh = (key == 't') ? !showThresh : showThresh;
    ballDetection = (key == 'd') ? !ballDetection : ballDetection;
    showBlur = (key == 'b') ? !showBlur : showBlur;

    // Cycle through image indexes
    imageIndex = (key == '>' || key == '.') ? std::min(imageIndex + 1, static_cast<int>(images.size()-1)) : imageIndex;
    imageIndex = (key == '<' || key == ',') ? std::max(imageIndex - 1, 0) : imageIndex;


    // Save if 's' is pressed, and a file was given to output to.
    if (key == 's' && outputFile != "") {
      cv::FileStorage storage(outputFile, cv::FileStorage::WRITE);
      if (storage.isOpened()) {
        storage << "Threshold" << threshold;
      } else {
        std::cerr << "Error opening output file: '" << outputFile << "'\n";
        break;
      }
      storage.release();
    }

    // Exit the tool
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