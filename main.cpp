#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

vector<Rect> process_output(Mat output, float score_treshold = 0.6, float nms_treshold = 0.5)
{
  float *data = (float*) output.data;
  vector<Rect> rects;
  vector<float> scores;
  for (size_t i = 0; i < output.total(); i += 5) {
    auto score = data[i + 4];
    if (score < score_treshold)
      continue;

    auto w = data[i + 2];
    auto h = data[i + 3];
    auto x = max(0.f, data[i] - w / 2);
    auto y = max(0.f, data[i + 1] - h / 2);
    rects.push_back(Rect(x, y, w, h));
    scores.push_back(score);
  }

  vector<int> indexes;
  dnn::NMSBoxes(rects, scores, score_treshold, nms_treshold, indexes);

  vector<Rect> bboxes;
  for (auto i : indexes) bboxes.push_back(rects[i]);
  return bboxes;
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
    auto blob_params = dnn::Image2BlobParams(1.0 / 255.0, Size(640, 640), Scalar(), false, CV_32F, dnn::DNN_LAYOUT_NCHW, dnn::DNN_PMODE_LETTERBOX, 0.0);
    auto blob = dnn::blobFromImageWithParams(frame, blob_params);
    model.setInput(blob);
    auto model_out = model.forward();
    const int new_shape[] = {model_out.size[1], model_out.size[2]};
    model_out = model_out.reshape(0, 2, new_shape).t();
    auto bboxes = process_output(model_out);
    for (auto bbox : bboxes) {
      auto rect = blob_params.blobRectToImageRect(bbox, frame.size());
      rectangle(frame, rect.tl(), rect.br(), Scalar(0, 0, 255), 3);
    }

    imshow("Live", frame);
    if (waitKey(5) >= 0)
        break;
  }

  return 0;
}
