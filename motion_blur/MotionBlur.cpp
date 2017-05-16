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

#define KEY_ESC 27

cv::Mat img;

const int iteration_slider_max = 20;
int iteration_slider;

const int x_slider_max = 10;
int x_slider;

const int y_slider_max = 10;
int y_slider;

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

  cv::copyMakeBorder(*img, img_expanded, byt, byb, bxt, bxb, cv::BORDER_REPLICATE);

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

    cv::warpAffine(img_expanded, shift, M, img_expanded.size(),
                   cv::INTER_LINEAR, cv::BORDER_REPLICATE);
    cv::addWeighted(*dst, i_norm, shift, 1 - i_norm, WEIGHT_GAMMA,
                    *dst);
    //cv::imshow("step", *dst);
    //cv::waitKey();
  }
  return dst;
}

void on_trackbar( int, void* )
{
  double dx = (double) x_slider / (double) x_slider_max;
  double dy = (double) y_slider / (double) y_slider_max;
  int n = iteration_slider + 1;
  printf("n: %d;x, y: %f %f\n",n, dx, dy);
  cv::Mat* dst = get_blured(&img, n, dx, dy);
  cv::imshow( "motion blur", *dst );
  delete dst;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "usage: ./MotionBlur image.png" << endl;
    return 0;
  }

  cv::namedWindow("motion blur", 1);

  cv::createTrackbar( "Iterations", "motion blur", 
    &iteration_slider, 
    iteration_slider_max, 
    on_trackbar );

  cv::createTrackbar( "x", "motion blur", 
    &x_slider, 
    x_slider_max, 
    on_trackbar );

  cv::createTrackbar( "y", "motion blur", 
    &y_slider, 
    y_slider_max, 
    on_trackbar );

  img = cv::imread(argv[1], cv::IMREAD_UNCHANGED);

  cv::imshow("motion blur", img);

  bool exit = false;

  while(!exit)
  {
    int key = (cv::waitKey() & 0xEFFFFF);
    switch(key)
    {
      case KEY_ESC:
        exit = true;
        break;
    }
  }

  int n = iteration_slider + 1;
  double dx = (double) x_slider / (double) x_slider_max;
  double dy = (double) y_slider / (double) y_slider_max;

  cv::Mat* result = get_blured(&img, n, dx, dy);
  cv::imwrite("res.png", *result);

  delete result;

  return 0;
}