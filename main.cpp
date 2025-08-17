#include <ctime>
#include <print>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

#define shift_args(argc, argv) ((argc)--, *(argv)++)

bool DEBUG = false;

struct Human {
  size_t id;
  Rect box;
  time_t appearance;
};

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

int main(int argc, const char **argv)
{
  shift_args(argc, argv); // program name
  const char *source = "";
  while (argc > 0) {
    auto arg = shift_args(argc, argv);
    if (strcmp(arg, "-D") == 0) DEBUG = true;
    else source = arg;
  }

  if (strcmp(source, "") == 0) {
    cerr << "ERROR: please specify device, URL or file for frame processing" << endl;
    return 1;
  }

  VideoCapture cap;
  auto device_id = atoi(source);
  if (device_id > 0 || strcmp(source, "0") == 0) {
    cap = VideoCapture(device_id);
    if (!cap.isOpened()) {
      cerr << "ERROR: can't open video capture for " << source << endl;
      return 1;
    }
  } else {
    cap = VideoCapture(source);
    if (!cap.isOpened()) {
      cerr << "ERROR: can't open video capture for " << source << endl;
      return 1;
    }
  }

  auto frames_count = cap.get(CAP_PROP_FRAME_COUNT);
  if (frames_count == 0) frames_count = 25;
  auto delay_frames = frames_count == 1 ? -1 : 5;
  auto model = dnn::readNetFromONNX("best.onnx");
  size_t id = 0;
  auto humans = vector<Human>();
  auto frame = Mat();
  auto writer = VideoWriter();
  time_t writer_start;
  while (cap.read(frame)) {
    auto blob_params = dnn::Image2BlobParams(1.0 / 255.0, Size(640, 640), Scalar(), false, CV_32F, dnn::DNN_LAYOUT_NCHW, dnn::DNN_PMODE_LETTERBOX, 0.0);
    auto blob = dnn::blobFromImageWithParams(frame, blob_params);
    model.setInput(blob);
    auto model_out = model.forward();
    const int new_shape[] = {model_out.size[1], model_out.size[2]};
    model_out = model_out.reshape(0, 2, new_shape).t();
    auto bboxes = process_output(model_out);
    auto cur_time = time(NULL);
    for (auto bbox : bboxes) {
      auto rect = blob_params.blobRectToImageRect(bbox, frame.size());
      float max_iou = 0.0;
      Human *cur_human;
      for (auto &human : humans) {
        auto iou = float((rect & human.box).area()) / float((rect | human.box).area());
        if (iou >= max_iou) {
          max_iou = iou;
          cur_human = &human;
        }
      }

      if (max_iou >= 0.5) {
        cur_human->box = rect;
        cur_human->appearance = cur_time;
      }
      else humans.push_back((Human) {id++, rect, time(NULL)});
    }

    for (auto it = humans.begin(); it < humans.end(); it += 1) {
      if (difftime(cur_time, it->appearance) > 5) {
        humans.erase(it);
        it -= 1;
        continue;
      }

      if (DEBUG) {
        rectangle(frame, it->box.tl(), it->box.br(), Scalar(0, 0, 255), 3); 
        putText(frame, to_string(it->id), it->box.tl(), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 0, 0));
      }
    }

    if (!writer.isOpened() && humans.size() > 0) {
      writer_start = time(NULL);
      writer.open(
        std::format("{}.avi", cur_time),
        VideoWriter::fourcc('M','J','P','G'),
        frames_count,
        frame.size()
      );
    }

    writer.write(frame);

    if (
      writer.isOpened() &&
      (humans.size() == 0 || difftime(cur_time, writer_start) >= 60)
    ) writer.release();

    // TODO: infinite loop without DEBUG
    if (DEBUG) {
      imshow("Live", frame);
      if (waitKey(delay_frames) == 81)
          break;
    }
  }

  if (writer.isOpened()) writer.release();

  if (DEBUG) {
    println("{}", humans.size());
  }

  return 0;
}
