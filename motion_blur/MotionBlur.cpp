#include <math.h>
#include <iostream>
#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

#define NOT_SAME_TYPE -1

#define WEIGHT_ALPHA 0.7
#define WEIGHT_BETA 0.3
#define WEIGHT_GAMMA 0

// the matrix with blured image is created on heap (new)
// within this function because I dont know blured image
// size until I call get_sequence()
// Pointer to blured image matrix is returned
cv::Mat* get_blured(const cv::Mat* img, int n, double x = 1.0, double y = 0.0, double a = 1.0) {
  cv::Mat img_expanded;

  int dtx = img->cols / (n * 2);
  int dty = img->rows / (n * 2);
  
  int bxb = (x > 0) ? n * abs(x) * dtx * a : 0;
  int byb = (y > 0) ? n * abs(y) * dty * a : 0;

  int bxt = (x < 0) ? n * abs(x) * dtx * a : 0;
  int byt = (y < 0) ? n * abs(y) * dty * a : 0;

  cv::copyMakeBorder(*img, img_expanded, byt, byb, bxt, bxb, cv::BORDER_CONSTANT);

  cv::Mat* dst = new cv::Mat(img_expanded.size(), img_expanded.type());

  cv::Mat temp(img_expanded.size(), img_expanded.type());

  img_expanded.copyTo(*dst);

  for (int i = 0; i < n; i++) {
    double posx = (i + 1) * x * dtx * a;
    double posy = (i + 1) * y * dty * a;

    double m[2][3] = {{1.0, 0.0, posx},
                      {0.0, 1.0, posy}};

    cv::Mat M = cv::Mat(2, 3, CV_64F, m);
    cv::Mat shift = cv::Mat(img_expanded.size(), img_expanded.type());

    double i_norm = (double) i / (double) n;
    printf("step %d: i norm = %f\n",i, i_norm); 

    cv::warpAffine(img_expanded, shift, M, img_expanded.size(),
                   cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);
    cv::addWeighted(*dst, i_norm, shift, 1 - i_norm, WEIGHT_GAMMA,
                    *dst);
    cv::imshow("step", *dst);
    cv::waitKey();
  }
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
  int n = 5;

  if (argc < 2) {
    cout << "usage: ./MotionBlur image.png" << endl;
    return 0;
  }

  if(argc > 2)
  {
    n = atoi(argv[2]);
  }

  //cv::Mat img = cv::imread(argv[1], cv::IMREAD_UNCHANGED);
  cv::Mat img = cv::imread(argv[1]);

  cv::Mat bg = cv::imread("bg.png");

  // cout << img.type() << endl;

  cv::Mat* dst = get_blured(&img, n, 1, 0.5);

  // dst->copyTo(bg(cv::Rect(0,0,dst->cols, dst->rows)));

  cout << img.size() << endl;

  cv::imshow("res", *dst);
  cv::imwrite("res.png", *dst);

  cv::waitKey();

  delete dst;

  return 0;
}