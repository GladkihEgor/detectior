#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

Mat resize_with_padding(Mat src, Size target_size)
{
  if (src.empty()) throw invalid_argument("Got empty image for resize with padding");
  if (target_size.width == 0 || target_size.height == 0) throw invalid_argument("Size for resize can't be 0");
  if (src.size() == target_size) return src; // TODO: this change src image
  
  auto ratio_size = src.rows / src.cols < target_size.height / target_size.width
    ? Size(target_size.width, min(target_size.height, target_size.width * src.rows / src.cols))
    : Size(min(target_size.width, target_size.height * src.cols / src.rows), target_size.height);

  Mat dst = Mat::zeros(target_size, src.type());
  auto roi = dst(Rect(0, 0, ratio_size.width, ratio_size.height));
  resize(src, roi, ratio_size);
  return dst;
}

int main()
{
  // auto source = "./chel.jpg";
  // auto source = "./big_buck_bunny_480p_h264.mov";
  // auto source = "rtsp://localhost:8554/bunny";
  auto source = 1;
  auto cap = VideoCapture(source);
  if (!cap.isOpened()) {
    cerr << "ERROR: can't open video capture for " << source << endl;
    return 1;
  }

  auto model = dnn::readNetFromONNX("best.onnx");

  auto frame = Mat();
  while (cap.read(frame)) {
    auto new_frame = resize_with_padding(frame, Size(640, 640));
    auto blob = dnn::blobFromImage(new_frame, 1.0 / 255.0, Size(), Scalar(), false, false, CV_32F);
    model.setInput(blob);
    auto model_out = model.forward();
    cout << model_out.total() << endl;
    imshow("Live", new_frame);
    if (waitKey(5) >= 0)
        break;
  }

  return 0;
}
