#include <iostream>
#include <string>
#include <vector>
#include <math.h>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

static void radioButtonCallback(int state, void* data) {
  *reinterpret_cast<bool*>(data) = (state == 0) ? false : true; 
}

int main(int argc, char** argv) {

  //****************************************************************************
  // Argument Parsing
  //****************************************************************************
  
  // Keys for argument parsing
  const std::string keys = 
  "{ h ? help usage |   | prints this message             }"
  "{ c camera       | 0 | Camera id used for thresholding }";

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
  int cameraID = parser.get<double>("camera"); // camera id

  // Cheack for errors
  if (!parser.check()) {
    parser.printErrors();
    return 0;
  }

  //****************************************************************************
  // GUI setup
  //****************************************************************************

  // Threshold variables
  std::array<int, 3> low =  {  0,   0,   0};
  std::array<int, 3> high = {180, 255, 255};

  // Create window
  cv::namedWindow("HSV Tunning");

  // Create Trackbars
  cv::createTrackbar("Low H",  "HSV Tunning", &low[0],  180);
  cv::createTrackbar("High H", "HSV Tunning", &high[0], 180);
  cv::createTrackbar("Low S",  "HSV Tunning", &low[1],  255);
  cv::createTrackbar("High S", "HSV Tunning", &high[1], 255);
  cv::createTrackbar("Low V",  "HSV Tunning", &low[2],  255);
  cv::createTrackbar("High V", "HSV Tunning", &high[2], 255);

  //****************************************************************************
  // Camera Initialization
  //****************************************************************************

  // Camera
  cv::VideoCapture capture(cameraID);

  // Check camera data
  if (!capture.isOpened()) {
    std::cerr << "Could access camera\n";
    return 0;
  }

  bool showThresh = true, useBallDetection = false;
  cv::Mat frame, blur, hsv, thresh, erode0, dilate, erode1, display;
  while (true) {

    // Get the next frame from the camera
    capture >> frame;

    // Check camera data
    if (frame.empty()) {
      std::cerr << "Lost connection to camera\n";
      break;
    }

    //**************************************************************************
    // Threshold Image
    //**************************************************************************

    // Blur frame to remove image noise
    cv::GaussianBlur(frame, blur, {15, 15}, 5);

    // Threshold the image in the hsv color space.
    cv::cvtColor(blur, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, low, high, thresh);

    // Erode image to remove small particles.
    // Then dilate tofill any holes.
    // Finaly erode back down to the origional size. 
    cv::erode(thresh, erode0, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15)));
    cv::dilate(erode0, dilate, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 30)));
    cv::erode(dilate, erode1, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15)));

    // Conditionaly display a thresheld view 
    if (showThresh) {
      cv::cvtColor(erode1, display, cv::COLOR_GRAY2BGR);
    } else {
      frame.copyTo(display);
    }

    //**************************************************************************
    // Ball Detection
    //**************************************************************************

    if (useBallDetection) {
      // Find contours in the image
      std::vector<std::vector<cv::Point>> contours;
      cv::findContours(erode1, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

      for (auto contour : contours) {
        // Find aproximate circle
        cv::Point2f center;
        float radius;
        cv::minEnclosingCircle(contour, center, radius);

        // Amount of enclosing circle that is filled by the contour.
        // (A sort of circularness calculation. The closer to 1 the better) 
        double percentFull = cv::contourArea(contour) / (radius * radius * M_PI);

        // Filters for aertain percent full, and minimum size
        if (cv::contourArea(contour) > 20 && percentFull > 0.60) {
          // Draw the estimated circle
          cv::circle(display, center, (int) radius, {0,0,255}, 3);
        }
      }
    }

    // Show the frames.
    cv::imshow("HSV Tunning", display);

    // Parse key presses.
    int key = cv::waitKey(30);

    showThresh = (key == 't') ? !showThresh : showThresh;
    useBallDetection = (key == 'b') ? !useBallDetection : useBallDetection;

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