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
               "\nTool to genrate a Target\n");

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

    // It is easier to parse formated data with the C function as oposed to cin,
    // but to get it to work reliably, I found it is best to pull it to a string
    // and then parse the string rather than parsing the raw input.
    std::cout << "Enter Ball center: ";
    std::string input;
    std::cin >> input;
    int numParsed = sscanf(input.c_str(), "(%f,%f,%f)", &ball.center.x, &ball.center.y, &ball.center.z);
    
    if (numParsed != 3) {
      ball.center = {0,0,0};
    }

    std::cout << "\n";

    // Once completed, Write the ball to a file
    if (outputFile != "") {
      cv::FileStorage storage(outputFile, cv::FileStorage::WRITE);
      if (storage.isOpened()) {
        storage << "Ball" << ball;
      }
      storage.release();
    }

  } else {
    std::vector<rv::Target> targets;
    while (true) {
      rv::Target target;

      std::cout << "\nEnter the target name: ";
      std::cin >> target.name;

      // TODO: Fix this god auful Interface
      std::cout << "Enter target points (Type 'done!' when finnished):\n\n";

      while (true) {
        // It is easier to parse formated data with the C function as oposed to cin,
        // but to get it to work reliably, I found it is best to pull it to a string
        // and then parse the string rather than parsing the raw input.
        std::string input;
        std::cin >> input;

        // Cheack if the user is done
        if (input == "done!" || input == "done") {
          break;
        }

        cv::Point2f point;
        int numParsed = sscanf(input.c_str(), "(%f,%f)", &point.x, &point.y);

        // If the parser found both coordinates add it to the shape
        if (numParsed == 2) {
          target.shape.push_back(point);
        } else {
          std::cout << "Invalid Format! Point will not be added to target.\n";
        }
      }

      std::cout << "\n";
      targets.push_back(target);

      // Ask the user to add another target
      std::string input;
      std::cout << "Add another Target (yes or no): ";
      std::cin >> input;

      // If they sad yes start adding another, otherwise exit the loop
      if (input == "yes" || input == "y") {
        continue;
      } else {
        break;
      }
    }

    // Write the targets to a file
    if (outputFile != "") {
      cv::FileStorage storage(outputFile, cv::FileStorage::WRITE);
      if (storage.isOpened()) {
        storage << "Target" << targets;
      }
    }
  }
}