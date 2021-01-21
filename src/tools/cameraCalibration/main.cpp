#include <iostream>
#include <filesystem>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>

#include "rambunctionVision/imageProcessing.hpp"
#include "rambunctionVision/camera.hpp"

void findChessboardPointsInImages(std::vector<cv::Mat>& images, std::vector<std::vector<cv::Point2f>>& chessboardPoints, cv::Size chessboardSize);

bool findChessboardPointsInVideo(cv::VideoCapture& capture, std::vector<std::vector<cv::Point2f>>& chessboardPoints, cv::Size chessboardSize);

std::vector<std::vector<cv::Point3f>> getChessboardPoints(cv::Size chessboardSize, double Squaresize, int n);

int main (int argc, char** argv) {
  // Keys for argument parsing
  const std::string keys = 
  "{ h ? help usage |       | prints this message             }"
  "{ c camera       |   0   | Camera id used for thresholding }"
  "{ i images       |       | Optional images for HSV Tunning }"
  "{ squareSize     |   25  | Size of each chessboard square  }"
  "{ chessboardSize | (9,6) | Dimensions of the chessboard    }"
  "{ in input       |       | Input file                      }"
  "{ out output     |       | Output file                     }";

  // Parser object
  cv::CommandLineParser parser(argc, argv, keys);
  parser.about("\nvision2021 v0.0.0 cameraCalibration"
               "\nTool to calibrate a camera\n");

  // Show help if help is flagged.
  if (parser.has("help")) {
    parser.printMessage();
    return 0;
  }

  // Get arguments from the parser
  int cameraID = parser.get<double>("camera");
  bool useImages = parser.has("images");
  std::string pathToImages = parser.get<std::string>("images");
  double squareSize = parser.get<int>("squareSize");
  std::string inputFile = parser.get<std::string>("input");
  std::string outputFile = parser.get<std::string>("output");

  cv::Size chessboardSize;
  if (sscanf(parser.get<std::string>("chessboardSize").c_str(), "(%d,%d)", &chessboardSize.width, &chessboardSize.height) != 2) {
    std::cerr << "Invalid format for argument 'chessboardSize'\n";
    return 0;
  }

  // Cheack for errors
  if (!parser.check()) {
    parser.printErrors();
    return 0;
  }

  cv::FileStorage fileStorage;
  rv::Camera camera;

  if (inputFile != "" && std::filesystem::exists(inputFile)) {
    fileStorage.open(inputFile, cv::FileStorage::READ);
    if (fileStorage.isOpened()) {
      fileStorage["Camera"] >> camera;
    }
  }

  cv::VideoCapture capture;
  std::vector<cv::Mat> images;
  std::vector<std::vector<cv::Point2f>> chessboardPoints;
  cv::Size imageSize;

  if (useImages) {
    if (!rv::extractImagesFromDirectory(pathToImages, images)) {
      std::cerr << "File: '" << pathToImages << "' does not exist\n";
      return 0;
    }

    if (images.empty()) {
      std::cerr << "No images could be found at" << pathToImages << "\n";
      return 0;
    }

    imageSize = {images[0].rows, images[0].cols};
    for (auto& image : images) {
      if (imageSize != cv::Size{image.rows, image.cols}) {
        std::cerr << "Images at directory: '" << pathToImages << "' have diffrent sizes\n";
        return 0;
      }
    }

    findChessboardPointsInImages(images, chessboardPoints, chessboardSize);

  } else {
    capture.open(cameraID);
    imageSize = {static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT)), static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH))};

    if (!findChessboardPointsInVideo(capture, chessboardPoints, chessboardSize)) {
      return 0;
    }

    capture.release();
  }

  if (chessboardPoints.empty()) {
    std::cerr << "No points were found\n";
    return 0;
  }

  cv::Mat rvecs, tvecs;
  cv::calibrateCamera(getChessboardPoints(chessboardSize, squareSize, chessboardPoints.size()), chessboardPoints, imageSize, camera.matrix, camera.distortion, rvecs, tvecs);

  if (inputFile != "" && std::filesystem::exists(inputFile)) {
    fileStorage.open(inputFile, cv::FileStorage::READ);
    if (fileStorage.isOpened()) {
      fileStorage["Camera"] >> camera;
    }
  }

  if (!useImages) {
    capture.open(cameraID);

    if (!capture.isOpened()) {
      std::cerr << "Could not find camera.\n";
      return 0;
    }
  }

  int imageIndex = 0;
  cv::Mat image, display;
  bool undistort = true;
  while (true) {
    if (useImages) {
      image = images[imageIndex];
    } else {
      capture >> image;
    }

    if (image.empty()) {
      std::cerr << "Image or camera not found\n";
      break;
    }

    std::vector<cv::Point2f> imagePoints;
    bool found = cv::findChessboardCorners(image, chessboardSize, imagePoints);

    if (found) {
      cv::drawChessboardCorners(image, chessboardSize, imagePoints, found);
    }

    if (undistort) {
      cv::undistort(image, display, camera.matrix, camera.distortion);
    } else {
      image.copyTo(display);
    }

    cv::imshow("Show Calibrated Camera", display);

    char key = cv::waitKey(30);

    undistort = (key == 'd') ? !undistort : undistort;

    if (key == 's' && outputFile != "" ) {
      fileStorage.open(outputFile, cv::FileStorage::WRITE);
      if (fileStorage.isOpened()) {
        fileStorage << "Camera" << camera;
      }
      break;
    }

    if (key == 'q' | key == 27) {
      break;
    }

  }
  cv::waitKey(1);
  cv::destroyAllWindows();
  cv::waitKey(1);
  capture.release();
  return 0;
}

void findChessboardPointsInImages(std::vector<cv::Mat>& images, std::vector<std::vector<cv::Point2f>>& chessboardPoints, cv::Size chessboardSize) {
  chessboardPoints.reserve(images.size());
  for (auto& image : images) {
    std::vector<cv::Point2f> imagePoints;
    if(cv::findChessboardCorners(image, chessboardSize, imagePoints)) {
      chessboardPoints.push_back(imagePoints);
    }
  }
}

bool findChessboardPointsInVideo(cv::VideoCapture& capture, std::vector<std::vector<cv::Point2f>>& chessboardPoints, cv::Size chessboardSize) {
  if (!capture.isOpened()) {
    std::cerr << "Could not find camera\n";
    return false;
  }
  
  cv::Mat image;
  while (true) {
    capture >> image;

    if (image.empty()) {
      std::cerr << "Lost connection to camera\n";
      return false;
    }

    std::vector<cv::Point2f> imagePoints;
    bool found = cv::findChessboardCorners(image, chessboardSize, imagePoints);

    if (found) {
      cv::drawChessboardCorners(image, chessboardSize, imagePoints, found);
    }

    char key = cv::waitKey(30);

    if (key == ' ' && found) {
      image = cv::Mat(image.rows, image.cols, CV_8UC1, 255);
      chessboardPoints.push_back(imagePoints);
    }

    cv::imshow("Find Chessboard Points", image);
 
    if (key == 'q' | key == 27) {
      break;
    }
  }
  cv::waitKey(1);
  cv::destroyAllWindows();
  capture.release();
  cv::waitKey(1);
  return true;
}

std::vector<std::vector<cv::Point3f>> getChessboardPoints(cv::Size chessboardSize, double squareSize, int n) {
  std::vector<cv::Point3f> chessboardPoints;
  chessboardPoints.reserve(chessboardSize.area());
  for (int h = 0; h < chessboardSize.height; h++) {
    for (int w = 0; w < chessboardSize.width; w++) {
      chessboardPoints.push_back({static_cast<float>(squareSize * h), static_cast<float>(squareSize * w), 0});
    }
  }

  return std::vector<std::vector<cv::Point3f>>(n, chessboardPoints);
}