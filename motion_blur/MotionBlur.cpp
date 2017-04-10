#include <math.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

#define NOT_SAME_TYPE -1

#define WEIGHT_ALPHA 0.8
#define WEIGHT_BETA 0.2
#define WEIGHT_GAMMA 0

int get_sequence(const cv::Mat* img, int n, cv::Mat** seq, double x, double y) {
  cv::Mat img_expanded;
  int rows = img->rows;
  int cols = img->cols;

  int bx = (n * abs(x));
  int by = (n * abs(y));

  cv::copyMakeBorder(*img, img_expanded, 0, by, 0, bx, cv::BORDER_CONSTANT,
                     cv::Scalar(0, 0, 0, 0));

  for (int i = 0; i < n; i++) {
    double m[2][3] = {{1.0, 0.0, i * x}, {0.0, 1.0, i * y}};
    cv::Mat M = cv::Mat(2, 3, CV_64F, m);
    seq[i] = new cv::Mat(img_expanded.size(), img_expanded.type());
    cv::warpAffine(img_expanded, *seq[i], M, img_expanded.size(),
                   cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);
  }

  return 0;
}

// the matrix with blured image is created on heap (new)
// within this function because I dont know blured image
// size until I call get_sequence()
// Pointer to blured image matrix is returned
cv::Mat* get_blured(const cv::Mat* img, int n, double x = 8.0, double y = 4.0) {
  cv::Mat** seq = new cv::Mat*[n];
  get_sequence(img, n, seq, x, y);

  cv::Mat* dst = new cv::Mat(seq[0]->size(), seq[0]->type());

  seq[0]->copyTo(*dst);

  for (int i = 0; i < n; i++) {
    cv::addWeighted(*dst, WEIGHT_ALPHA, *seq[i], WEIGHT_BETA, WEIGHT_GAMMA,
                    *dst);
  }

  for (int i = 0; i < n; i++) {
    delete seq[i];
  }

  delete seq;

  return dst;
}

// WIP
int compose(const cv::Mat* fg, cv::Mat* bg, int x, int y) {
  // if(fg->channels() != 3)
  // cv::cvtColor(*fg, *fg, CV_GRAY2RGB);

  if (bg->channels() != 4) cv::cvtColor(*bg, *bg, CV_RGB2RGBA);

  int rows = fg->rows;
  int channels = fg->channels();
  int cols = fg->cols * channels;

  for (int i = 0; i < rows; i++) {
    uchar* row_bg = bg->ptr<uchar>(i + y);
    const uchar* row_fg = fg->ptr<uchar>(i);
    for (int j = 0; j < cols; j += channels) {
      int r = row_fg[j];
      int g = row_fg[j + 1];
      int b = row_fg[j + 2];

      if (r > 10 && g > 10 && b > 10) {
        row_bg[j + x] = r;
        row_bg[j + x + 1] = g;
        row_bg[j + x + 2] = b;
        row_bg[j + x + 3] = row_fg[j + 3];
      } else
        row_bg[j + x + 3] = 255;
    }
  }
}

int main(int argc, char** argv) {
  int n = 15;

  if (argc < 2) {
    cout << "usage: ./MotionBlur image.png" << endl;
  }

  cv::Mat img = cv::imread(argv[1], cv::IMREAD_UNCHANGED);
  cv::Mat bg = cv::imread("bg.png");

  // cout << img.type() << endl;

  cv::Mat* dst = get_blured(&img, n, -4, 0);

  // dst->copyTo(bg(cv::Rect(0,0,dst->cols, dst->rows)));

  // cv::imwrite("res.png", bg);
  cv::imwrite("res.png", *dst);

  delete dst;

  return 0;
}