#include <iostream>

#include <opencv2/core.hpp>

#include <rambunctionVision/contourProcessing.hpp>

int main (int argc, char** argv) {
  // Keys for argument parsing
  const std::string keys = 
  "{ h ? help usage | | prints this message             }"
  "{ b ball         | | Whether a ball should be made   }"
  "{ out output     | | Output File                     }";

  // Parser object
  cv::CommandLineParser parser(argc, argv, keys);
  parser.about("\nvision2021 v0.0.0 cameraCalibration"
               "\nTool to calibrate a camera\n");

  bool makeBall = parser.has("ball");
  std::string outputFile = parser.get<std::string>("output");

  // Cheack for errors
  if (!parser.check()) {
    parser.printErrors();
    return 0;
  }

  if (makeBall) {
    rv::Ball ball;

    std::cout << "\nEnter Ball radius: ";
    std::cin >> ball.radius;

    std::cout << "Enter Ball center: ";
    std::string input;
    std::cin >> input;
    if (sscanf(input.c_str(), "(%f,%f,%f)", &ball.center.x, &ball.center.y, &ball.center.z) != 3) {
      ball.center = {0,0,0};
    }

    std::cout << "\n";

    if (outputFile != "") {
      cv::FileStorage storage(outputFile, cv::FileStorage::WRITE);
      if (storage.isOpened()) {
        storage << "Ball" << ball;
      }
    }

  } else {

    std::vector<rv::Target> targets;

    while (true) {
      rv::Target target;

      std::cout << "\nEnter the target name: ";
      std::cin >> target.name;

      std::cout << "Enter target points (Type 'done!' when finnished):\n\n";

      while (true) {
        std::string input;
        std::cin >> input;

        if (input == "done!" || input == "done") {
          break;
        }

        cv::Point2f point;
        if (sscanf(input.c_str(), "(%f,%f)", &point.x, &point.y) == 2) {
          target.shape.push_back(point);
        } else {
          std::cout << "Invalid Format! Point will not be added to target.\n";
        }
      }

      std::cout << "\n";

      targets.push_back(target);

      std::string input;
      std::cout << "Add another Target (yes or no): ";
      std::cin >> input;

      if (input == "yes" || input == "y") {
        continue;
      } else {
        break;
      }
    }

    if (outputFile != "") {
      cv::FileStorage storage(outputFile, cv::FileStorage::WRITE);
      if (storage.isOpened()) {
        storage << "Target" << targets;
      }
    }
  }
}