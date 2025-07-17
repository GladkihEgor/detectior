#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

int main()
{
  // auto source = "rtsp://localhost:8554/bunny";
  auto source = 1;
  auto cap = VideoCapture(source);
  if (!cap.isOpened()) {
    cerr << "ERROR: can't open video capture for " << source << endl;
    return 1;
  }

  auto frame = Mat();
  while (cap.read(frame)) {
    imshow("Live", frame);
    if (waitKey(5) >= 0)
        break;
  }

  return 0;
}
