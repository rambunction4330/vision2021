#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <filesystem>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "rambunctionVision/camera.hpp"
#include "rambunctionVision/conversions.hpp"
#include "rambunctionVision/contourProcessing.hpp"
#include "rambunctionVision/imageProcessing.hpp"
#include "rambunctionVision/drawing.hpp"

int main(int argc, char** argv) {

  //****************************************************************************
  // Argument Parsing
  //****************************************************************************
  
  // Keys for argument parsing (The flags you can set on the executable)
  const std::string keys = 
  "{ h ? help usage |   | prints this message                   }"
  "{ id cameraID    | 0 | Camera id used for thresholding       }"
  "{ i images       |   | Optional images for HSV Tunning       }"
  "{ b blur         |   | Whether to present a blur slider      }"
  "{ m morph        |   | Whether to present morphology sliders }"
  "{ in input       |   | Input file                            }"
  "{ out output     |   | Output file                           }"
  "{ camera         |   | Output file                           }"
  "{ target         |   | Output file                           }"
  "{ ball           |   | Output file                           }";

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
  int cameraID = parser.get<double>("cameraID");
  std::string pathToImages = parser.get<std::string>("images");
  bool useBlurSlider = parser.has("blur");
  bool useMorphSlider = parser.has("morph");
  std::string inputFile = parser.get<std::string>("input");
  std::string outputFile = parser.get<std::string>("output");
  std::string cameraFile = parser.get<std::string>("camera");
  std::string targetsFile = parser.get<std::string>("target");
  std::string ballFile = parser.get<std::string>("ball");

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
  if (inputFile != "") {
    if (std::filesystem::exists(inputFile)) {
      cv::FileStorage storage(inputFile, cv::FileStorage::READ);
      if (storage.isOpened()) {
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

  // -------------------------------
  // Extract camera and target data 
  // -------------------------------

  rv::Camera camera;

  if (cameraFile != "") {
    if (std::filesystem::exists(cameraFile)) {
      cv::FileStorage storage(cameraFile, cv::FileStorage::READ);
      if (storage.isOpened()) {
        storage["Camera"] >> camera;
      } else {
        std::cerr << "Error opening camera file: '" << cameraFile << "'\n";
        return 0;
      }
      storage.release();
    } else {
      std::cerr << "Could not find camera file: '" << cameraFile << "'\n";
      return 0;
    }
  }

  std::vector<rv::Target> targets;

  if (targetsFile != "") {
    if (std::filesystem::exists(targetsFile)) {
      cv::FileStorage storage(targetsFile, cv::FileStorage::READ);
      if (storage.isOpened()) {
        storage["Targets"] >> targets;
      } else {
        std::cerr << "Error opening targets file: '" << targetsFile << "'\n";
        return 0;
      }
      storage.release();
    } else {
      std::cerr << "Could not find targets file: '" << targetsFile << "'\n";
      return 0;
    }
  }

  rv::Ball ball;

  if (ballFile != "") {
    if (std::filesystem::exists(ballFile)) {
      cv::FileStorage storage(ballFile, cv::FileStorage::READ);
      if (storage.isOpened()) {
        storage["Ball"] >> ball;
      } else {
        std::cerr << "Error opening ball file: '" << ballFile << "'\n";
        return 0;
      }
      storage.release();
    } else {
      std::cerr << "Could not find ball file: '" << ballFile << "'\n";
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
  bool estimatePose = false;

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

      if (estimatePose && cameraFile != "" && ballFile != "") {
        std::vector<rv::BallPose> positions = rv::estimateBallPose(circles, ball, camera.matrix, camera.distortion);

        for (auto& position : positions) {
          rv::drawAxis(display, 5, camera.matrix, camera.distortion, position.rvec, position.tvec);
        }

      } else {
        // Draw all the circles
        for (auto& circle : circles) {
          cv::circle(display, circle.circle.center, circle.circle.radius, {0, 0, 255}, 4);
        }
      }
    }

    if (targetDetection) {
      cv::drawContours(display, contours, -1, { 255, 0, 0}, 2); 

      if (targetsFile != "") {
        std::vector<rv::TargetMatch> matches = rv::findTargets(contours, targets, 50, 5.0);

        std::vector<rv::TargetMatch> proccessedMatch = rv::matchTargetPoints(matches);

        for (auto& match : proccessedMatch) {
          cv::putText(display, match.target.name, match.shape[0], cv::FONT_HERSHEY_SIMPLEX, 1.0, {0,0,255}, 2);
          cv::drawContours(display, std::vector<std::vector<cv::Point>>(1, rv::convertToPoints<int>(match.shape)), -1, {0, 0, 255}, 4);
        }

        if (estimatePose && cameraFile != "" ) {
          std::vector<rv::TargetPose> positions = rv::estimateTargetPose(proccessedMatch, camera.matrix, camera.distortion);

          for (auto& position : positions) {
            rv::drawAxis(display, 5, camera.matrix, camera.distortion, position.rvec, position.tvec);
          }
        }
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
    ballDetection = (key == 'c') ? !ballDetection : ballDetection;
    targetDetection = (key == 'd') ? !targetDetection : targetDetection;
    estimatePose = (key == 'p') ? !estimatePose : estimatePose;
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