#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <ctime>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/calib3d.hpp>

#include <networktables/NetworkTable.h>
#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTableEntry.h>

#include <rambunctionVision/conversions.hpp>
#include <rambunctionVision/camera.hpp>
#include <rambunctionVision/imageProcessing.hpp>
#include <rambunctionVision/contourProcessing.hpp>

int main (int argc, char** argv) {
  
  //****************************************************************************
  // Argument Parsing
  //****************************************************************************
  
  // Keys for argument parsing (The flags you can set on the executable)
  const std::string keys = 
  "{ h ? help usage |   | prints this message                   }"
  "{ id cameraID    | 0 | Camera id used for thresholding       }"
  "{ c camera       |   | File holding camera calibration       }"
  "{ t thresholding |   | File holding image thresholding data  }"
  "{ b ball         |   | File with ball size data              }";

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
  std::string cameraFile = parser.get<std::string>("camera");
  std::string threshFile = parser.get<std::string>("thresholding");
  std::string ballFile = parser.get<std::string>("ball");

  // Cheack for errors
  if (!parser.check()) {
    parser.printErrors();
    return 0;
  }

  //****************************************************************************
  // Extract Data From Input Files
  //****************************************************************************

  // Variable holding camera calibration.
  rv::Camera camera;

  if (cameraFile != "") {
    if (std::filesystem::exists(cameraFile)) {
      cv::FileStorage storage(cameraFile, cv::FileStorage::READ);
      if (storage.isOpened()) {
        storage["Camera"] >> camera;
        if (camera.distortion.empty() || camera.matrix.empty()) {
          std::cerr << "Error extracting data from camera file: '" << cameraFile << "'\n";
          return 0;  
        }
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

  // Variable to hold any thresholding data.
  rv::Threshold threshold;

  // If a file was given, extract the data from that file
  if (threshFile != "") {
    if (std::filesystem::exists(threshFile)) {
      cv::FileStorage storage(threshFile, cv::FileStorage::READ);
      if (storage.isOpened()) {
        storage["Threshold"] >> threshold;
        if (threshold.openMatrix.empty() || threshold.closeMatrix.empty()) {
          std::cerr << "Error extracting data from camera file: '" << cameraFile << "'\n";
          return 0;  
        }
      } else {
        std::cerr << "Error opening input file: '" << threshFile << "'\n";
        return 0;
      }
      storage.release();
    } else {
      std::cerr << "Could not find input file: '" << threshFile << "'\n";
      return 0;
    }
  }

  rv::Ball ball;

  if (ballFile != "") {
    if (std::filesystem::exists(ballFile)) {
      cv::FileStorage storage(ballFile, cv::FileStorage::READ);
      if (storage.isOpened()) {
        storage["Ball"] >> ball;
        if (ball.radius > 500 || ball.radius < 0.001) {
          std::cerr << "Error extracting data from camera file: '" << cameraFile << "'\n";
          return 0;  
        }
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

  //****************************************************************************
  // Setup Camera
  //****************************************************************************


  // Camera for video tunning
  cv::VideoCapture capture(cameraID);

  // Check camera data
  if (!capture.isOpened()) {
    std::cerr << "Could access camera with id: '" << cameraID << "'\n";
    return 0;
  }

  //****************************************************************************
  // Network Tables Setup
  //****************************************************************************

  // Table structer
  //
  // root
  // | BallDetection
  // | | BallData
  // | | | numBalls
  // | | | ballSize
  // | | | sortMethod
  // | | | Ball0
  // | | | | tvec
  // | | | | x
  // | | | | y
  // | | | | z
  // | | | | rvec
  // | | | | roll
  // | | | | pitch
  // | | | | yaw
  // | | | | match
  // | | | | age
  // | | | Ball1
  // | | | Ball2
  // | | | Ball3
  // | | | ...
  // | | CameraData
  // | | | cameraID
  // | | | matrix
  // | | | distortion
  // | | | FPS
  // | | | rawFPS
  // | | | stream
  // | | | overlay
  // | | TimeingData
  // | | | captureTime
  // | | | threshTime
  // | | | contourTime
  // | | | matchTime
  // | | | poseTime
  // | | | networkTime
  // | | | totalTime

  
  // Initilize Network
  auto tableInstance = nt::NetworkTableInstance::GetDefault();
  auto cameraTable = tableInstance.GetTable("BallDetection/CameraData");
  auto ballTable = tableInstance.GetTable("BallDetection/BallData");
  auto timeTable = tableInstance.GetTable("BallDetection/TimeData");
  tableInstance.StartClientTeam(4330);
  tableInstance.StartDSClient();

  // Intilize Camera Data
  cameraTable->GetEntry("ID").SetDouble(cameraID);
  cameraTable->GetEntry("rawFPS").SetDouble(capture.get(cv::CAP_PROP_FPS));
  cameraTable->GetEntry("FPS").SetDouble(capture.get(cv::CAP_PROP_FPS));
  cameraTable->GetEntry("matrix").SetDoubleArray({camera.matrix.at<double>(0,0), camera.matrix.at<double>(1,0), camera.matrix.at<double>(2,0),
                                                  camera.matrix.at<double>(0,1), camera.matrix.at<double>(1,1), camera.matrix.at<double>(2,1),
                                                  camera.matrix.at<double>(0,2), camera.matrix.at<double>(1,2), camera.matrix.at<double>(2,2)});

  cameraTable->GetEntry("distortion").SetDoubleArray({camera.distortion.at<double>(0,0), camera.distortion.at<double>(0,1),  camera.distortion.at<double>(0,2), camera.distortion.at<double>(0,3), camera.distortion.at<double>(0,4)});

  // Initilize Ball Data
  ballTable->GetEntry("numBalls").SetDouble(0);
  ballTable->GetEntry("ballRadius").SetDouble(ball.radius);
  ballTable->GetEntry("sortMethod").SetString("Closest");

  // Initilize Time Data
  timeTable->GetEntry("captureTime").SetDouble(0);
  timeTable->GetEntry("threshTime").SetDouble(0);
  timeTable->GetEntry("contourTime").SetDouble(0);
  timeTable->GetEntry("matchTime").SetDouble(0);
  timeTable->GetEntry("poseTime").SetDouble(0);
  timeTable->GetEntry("networkTime").SetDouble(0);
  timeTable->GetEntry("totalTime").SetDouble(0);

  cv::Mat frame, thresh;
  while (true) {
    // Start of processing time to calculate frame rate.
    auto start = std::chrono::high_resolution_clock::now();

    // Get the next frame
    auto captureStart = std::chrono::high_resolution_clock::now();
    capture >> frame;

    // Check camera data.
    if (frame.empty()) {
      std::cerr << "Lost connection to camera\n";
      break;
    }

    std::chrono::duration<double> captureTime = std::chrono::duration_cast<std::chrono::microseconds>(captureStart - std::chrono::high_resolution_clock::now());

    // Threshold image.
    auto threshStart = std::chrono::high_resolution_clock::now();
    rv::thresholdImage(frame, thresh, threshold);
    std::chrono::duration<double> threshTime = std::chrono::duration_cast<std::chrono::microseconds>(threshStart - std::chrono::high_resolution_clock::now());

    // Find contours in the image for ball detection.
    auto contourStart = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    std::chrono::duration<double> contourTime = std::chrono::duration_cast<std::chrono::microseconds>(contourStart - std::chrono::high_resolution_clock::now());

    // Find all the contours that are sufficently circular to be balls.
    auto matchStart = std::chrono::high_resolution_clock::now();
    std::vector<rv::CircleMatch> circles = rv::findCircles(contours, 50, 0.60);
    std::chrono::duration<double> matchTime = std::chrono::duration_cast<std::chrono::microseconds>(matchStart - std::chrono::high_resolution_clock::now());

    // Estimate the ball's poition from the circles.
    auto poseStart = std::chrono::high_resolution_clock::now();
    std::vector<rv::BallPose> positions = rv::estimateBallPose(circles, ball, camera.matrix, camera.distortion);
    std::chrono::duration<double> poseTime = std::chrono::duration_cast<std::chrono::microseconds>(poseStart - std::chrono::high_resolution_clock::now());

    // Send data over the network
    auto networkStart = std::chrono::high_resolution_clock::now();
    ballTable->GetEntry("numBalls").SetDouble(positions.size());
    
    // TODO: Add actual data
    for (int i = 0; i < positions.size(); i++) {
      auto table = tableInstance.GetTable("BallDetection/BallData/Ball" + std::to_string(i));
      
      // Position data
      table->GetEntry("tvec").SetDoubleArray({positions[i].tvec.at<double>(0,0), positions[i].tvec.at<double>(0,1), positions[i].tvec.at<double>(0,2)});
      table->GetEntry("x").SetDouble(positions[i].tvec.at<double>(0,0));
      table->GetEntry("y").SetDouble(positions[i].tvec.at<double>(0,1));
      table->GetEntry("z").SetDouble(positions[i].tvec.at<double>(0,2));

      // Extract rotation from rvec
      cv::Vec3d rotation = cv::RQDecomp3x3(positions[i].rvec, cv::noArray(), cv::noArray());

      // Rotation data
      table->GetEntry("rvec").SetDoubleArray({positions[i].rvec.at<double>(0,0), positions[i].rvec.at<double>(0,1), positions[i].rvec.at<double>(0,2)});
      table->GetEntry("roll").SetDouble(rotation[0]);
      table->GetEntry("pitch").SetDouble(rotation[1]);
      table->GetEntry("yaw").SetDouble(rotation[2]);

      // Other info
      table->GetEntry("match").SetDouble(positions[i].circleMatch.match);
      std::time_t t = time(NULL);
      table->GetEntry("age").SetString(std::asctime(std::gmtime(&t)));
    }

    // Send time data
    timeTable->GetEntry("captureTime").SetDouble((captureTime.count() / 1000000));
    timeTable->GetEntry("threshTime").SetDouble((threshTime.count() / 1000000));
    timeTable->GetEntry("contourTime").SetDouble((contourTime.count() / 1000000));
    timeTable->GetEntry("matchTime").SetDouble((matchTime.count() / 1000000));
    timeTable->GetEntry("poseTime").SetDouble((poseTime.count() / 1000000));

    auto networkTime = std::chrono::duration_cast<std::chrono::microseconds>(networkStart - std::chrono::high_resolution_clock::now());
    timeTable->GetEntry("networkTime").SetDouble((networkTime.count() / 1000000));

    std::chrono::duration<double> totalTime = std::chrono::duration_cast<std::chrono::microseconds>(start - std::chrono::high_resolution_clock::now());
    timeTable->GetEntry("totalTime").SetDouble((totalTime.count() / 1000000));
    cameraTable->GetEntry("FPS").SetDouble(1 / (totalTime.count() / 1000000));
  }
}