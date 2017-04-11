#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#define NOT_8UC3_IMAGE -3
#define NOT_MATCHING_TYPE -2
#define NOT_MATCHING_SIZE -1

#define KEY_ESC 27
#define KEY_SPACE 32
#define KEY_ALT 233

#define KEY_UP 82
#define KEY_DOWN 84
#define KEY_LEFT 81
#define KEY_RIGHT 83

#define TARGET_STEP 8

using namespace std;

cv::Mat display;

void print_matrix(const cv::Mat* mat) {
  printf("Size: %d r %d c\n", mat->rows, mat->cols);
  printf("Type: %d, channels: %d\n", mat->type(), mat->channels());

  int rows = mat->rows;
  int channels = mat->channels();
  int cols = mat->cols * channels;

  for (int i = 0; i < rows; i++) {
    const uchar* row = mat->ptr<uchar>(i);
    for (int j = 0; j < cols; j += channels) {
      printf("[ ");
      for (int k = 0; k < channels; k++) {
        printf("c%d=%d, ", k, row[j + k]);
      }
      printf(" ], ");
    }
    printf("\n");
  }
}

cv::Mat* get_neighbours(const cv::Mat* mat, int x, int y, int size) {
  cv::Rect r(y - size / 2, x - size / 2, size, size);

  cv::Mat* res = NULL;
  uchar red = mat->at<uchar>(x, y);
  uchar green = mat->at<uchar>(x + 1, y);
  uchar blue = mat->at<uchar>(x + 2, y);

  if (0 <= r.x && 0 <= r.width && r.x + r.width <= mat->cols && 0 <= r.y &&
      0 <= r.height && r.y + r.height <= mat->rows) {
    res = new cv::Mat(*mat, r);
  } else {
    res = new cv::Mat(1, 1, CV_8UC3);
    res->setTo(cv::Scalar(red, green, blue));
  }

  return res;
}

/*
color:
0 - r
1 - g
2 - b
*/
int get_average(const cv::Mat* mat, char color) {
  if (mat->type() != CV_8UC3) return NOT_8UC3_IMAGE;

  int rows = mat->rows;
  int channels = mat->channels();
  int cols = mat->cols * channels;

  int sum = 0;

  for (int i = 0; i < rows; i++) {
    const uchar* row = mat->ptr<uchar>(i);
    for (int j = 0; j < cols; j += channels) {
      sum += row[j + color];
    }
  }

  sum = sum / (rows * mat->cols);
  return sum;
}

int get_kernel(uchar a) {

  if (a < 32) return 13;
  if (a < 64) return 11;
  if (a < 128) return 7;
  if (a < 196) return 3;
  return 1;
}

int get_kernel_by_depth(uchar a, uchar depth)
{
  int diff = abs(a - depth);

  if (diff < 32) return 1;
  if (diff < 64) return 3;
  if (diff < 128) return 7;
  if (diff < 196) return 11;
  return 13;
}

void depth_visualise(const cv::Mat* depth) {
  double min;
  double max;
  cv::minMaxIdx(*depth, &min, &max);
  cv::Mat adjMap;
  cv::convertScaleAbs(*depth, adjMap, 255 / max);
  cv::imwrite("depth_vis.png", adjMap);
}

int depth_blur(const cv::Mat *mat, const cv::Mat *depth, int x, int y,
               cv::Mat *res) {
  if (mat->rows != res->rows || mat->cols != res->cols)
    return NOT_MATCHING_SIZE;
  if (mat->type() != res->type()) return NOT_MATCHING_TYPE;
  if (mat->type() != CV_8UC3) return NOT_8UC3_IMAGE;

  int rows = mat->rows;
  int channels = mat->channels();
  int cols = mat->cols * channels;

  cv::Vec3b target_depth = depth->at<cv::Vec3b>(y, x);
  printf("target depth at (%d, %d) = %d\n", x, y, target_depth[0]);

  for (int i = 1; i < rows; i++) {
    const uchar* row_in = mat->ptr<uchar>(i);
    const uchar* row_depth = depth->ptr<uchar>(i);
    uchar* row_out = res->ptr<uchar>(i);
    for (int j = 3; j < cols; j += channels) {
      //int k = get_kernel(row_depth[j]);
      int k = get_kernel_by_depth(row_depth[j], target_depth[0]);
      cv::Mat* neigbours = get_neighbours(mat, i, j / channels, k);

      row_out[j] = get_average(neigbours, 0);
      row_out[j + 1] = get_average(neigbours, 1);
      row_out[j + 2] = get_average(neigbours, 2);

      delete neigbours;
    }
  }

  return 0;
}

void draw_target(cv::Mat * img, int x, int y)
{
  img->copyTo(display);
  cv::Point fp = cv::Point(x, y);
  printf("x=%d y=%d\n",x,y);
  circle(display, fp, 4, cv::Scalar(0,0,255), CV_FILLED, 8);
  cv::imshow("Blur window", display);
}

int main(int argc, char** argv) {
  if (argc < 3) {
    cout << "Usage: ./DepthBlur image.png depth.png" << endl;
    return 0;
  }

  cv::Mat image = cv::imread(argv[1]);
  cv::Mat depth = cv::imread(argv[2]);

  display = cv::Mat::zeros(image.rows, image.cols, CV_8UC3);
  image.copyTo(display);

  cv::namedWindow("Blur window", 1);

  int pointx = 0;
  int pointy = 0;

  cv::imshow("Blur window", display);

  int key = 0;
  bool exit = false;
  bool display_depth = false;

  while(!exit)
  {
    key = (cv::waitKey() & 0xEFFFFF);

    switch (key) {
    case KEY_ESC:
      exit = true;
      break;
    case KEY_SPACE:
      depth_blur(&image, &depth, pointx, pointy, &display);
      cv::imshow("Blur window", display);
      break;
    case KEY_DOWN:
      if(pointy >= display.rows)
        break;
      pointy = (pointy + TARGET_STEP);
      draw_target(&image, pointx, pointy);
      break;
    case KEY_UP:
      if(pointy <= 0)
        break;
      pointy = (pointy - TARGET_STEP);
      draw_target(&image, pointx, pointy);
      break;
    case KEY_LEFT:
      if(pointx <= 0)
        break;
      pointx = (pointx - TARGET_STEP);
      draw_target(&image, pointx, pointy);
      break;
    case KEY_RIGHT:
      if(pointx >= display.cols)
        break;
      pointx = (pointx + TARGET_STEP);
      draw_target(&image, pointx, pointy);
      break;
    case KEY_ALT:
      if(!display_depth)
      {
        cv::imshow("Blur window", depth);
        display_depth = true;
      }
      else
      {
        cv::imshow("Blur window", display);
        display_depth = false;
      }
    default:
      printf("key = %d\n", key);
      break;
    }
  }
  return 0;
}
