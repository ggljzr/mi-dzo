#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#define NOT_8UC3_IMAGE -3
#define NOT_MATCHING_TYPE -2
#define NOT_MATCHING_SIZE -1

#define KEY_ESC 27
#define KEY_SPACE 32
#define KEY_ENTER 10
#define KEY_ALT 233
#define KEY_S 115

#define KEY_UP 82
#define KEY_DOWN 84
#define KEY_LEFT 81
#define KEY_RIGHT 83

#define TARGET_STEP 8

#define SIGMA_COLOR 30
#define SIGMA_SPACE 30

using namespace std;

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

  if (0 <= r.x && 0 <= r.width && r.x + r.width <= mat->cols && 0 <= r.y &&
      0 <= r.height && r.y + r.height <= mat->rows) {
    res = new cv::Mat(*mat, r);
  } else {
    res = new cv::Mat(1, 1, CV_8UC3);
    res->setTo(mat->at<cv::Vec3b>(x, y));
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

int get_kernel_by_depth(uchar depth, uchar target_depth, double min_depth,
                        double max_depth) {

  double maxdiff = max_depth - min_depth;
  double depthn = ((double) depth - min_depth) / maxdiff;
  double target_depthn = ((double) target_depth - min_depth) / maxdiff;

  double diff = abs(depthn - target_depthn);

  if (diff < 0.2) return 1;
  if (diff < 0.5) return 3;
  if (diff < 0.7) return 7;
  if (diff < 0.9) return 11;
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

  double max_depth;
  double min_depth;

  cv::minMaxLoc(*depth, &min_depth, &max_depth);

  cv::Vec3b target_depth = depth->at<cv::Vec3b>(y, x);
  printf("target depth at (%d, %d) = %d\n", x, y, target_depth[0]);

  for (int i = 1; i < rows; i++) {
    const uchar* row_in = mat->ptr<uchar>(i);
    const uchar* row_depth = depth->ptr<uchar>(i);
    uchar* row_out = res->ptr<uchar>(i);
    for (int j = 3; j < cols; j += channels) {
      //int k = get_kernel(row_depth[j]);
      int k = get_kernel_by_depth(row_depth[j], target_depth[0], min_depth, max_depth);
      cv::Mat* neigbours = get_neighbours(mat, i, j / channels, k);

      row_out[j] = get_average(neigbours, 0);
      row_out[j + 1] = get_average(neigbours, 1);
      row_out[j + 2] = get_average(neigbours, 2);

      delete neigbours;
    }
  }

  return 0;
}

double gaussian(double z, double p)
{
  double n = (z * z) * (-1);
  double d = (2 * p * p);
  return (exp(n/d));
}

double euclid_dist(int x1, int y1, int x2, int y2)
{
  double dist = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
  return sqrt(dist);
}

double bilateral_filter_pixel(cv::Mat * mat, int pix_row, int pix_col,
                              uchar pix_val, int channel) {
  int rows = mat->rows;
  int channels = mat->channels();
  int cols = mat->cols * channels;

  double wp = 0;
  double sum = 0;

  for (int i = 0; i < rows; i++) {
    const uchar* row = mat->ptr<uchar>(i);
    for (int j = 0; j < cols; j += channels) {
      //tohle je blbost protoze ty souradnice i, j
      //nejsou v ty puvodni matici ale v tech neigbours;
      double dist = euclid_dist(pix_col, pix_row, (j / 3) + pix_col, i + pix_row);
      double spat_val = gaussian(dist, SIGMA_SPACE);
      uchar current_val = row[j + channel];
      double range_val = gaussian((double) pix_val - (double) current_val, SIGMA_COLOR);

      wp += spat_val * range_val;

      sum += spat_val * range_val * row[j + channel];
    }
  }

  return sum / wp;
}

int bilateral_filter(cv::Mat * img, cv::Mat * res)
{
  int rows = img->rows;
  int channels = img->channels();
  int cols = img->cols * channels;

  for(int i = 0; i < rows; i++)
  {
    uchar * row_img = img->ptr<uchar>(i);
    uchar * row_res = res->ptr<uchar>(i);
    for(int j = 0; j < cols; j+= channels)
    {
      cv::Mat * neigbours = get_neighbours(img, i, j / channels, 3);
      uchar r = row_img[j];
      uchar g = row_img[j + 1];
      uchar b = row_img[j + 2];

      double valr = bilateral_filter_pixel(neigbours, i, j / 3, r, 0);
      double valg = bilateral_filter_pixel(neigbours, i, j / 3, g, 1);
      double valb = bilateral_filter_pixel(neigbours, i, j / 3, b, 2);

      row_res[j] = (uchar) (valr);
      row_res[j + 1] = (uchar) (valg);
      row_res[j + 2] = (uchar) (valb);

      delete neigbours;
    }
  }

  return 0;
}

void draw_target(cv::Mat * img, int x, int y, cv::Mat * display)
{
  img->copyTo(*display);
  cv::Point fp = cv::Point(x, y);
  printf("x=%d y=%d\n",x,y);
  circle(*display, fp, 4, cv::Scalar(0,0,255), CV_FILLED, 8);
  cv::imshow("Blur window", *display);
}

int main(int argc, char** argv) {
  if (argc < 3) {
    cout << "Usage: ./DepthBlur image.png depth.png" << endl;
    return 0;
  }

  cv::Mat image = cv::imread(argv[1]);
  cv::Mat depth = cv::imread(argv[2]);
  cv::Mat display;
  cv::Mat * disp_src = &image;

  display = cv::Mat::zeros(image.rows, image.cols, CV_8UC3);
  image.copyTo(display);

  cv::namedWindow("Blur window", 1);

  int pointx = 0;
  int pointy = 0;

  printf("image rows = %d, cols = %d\n", image.rows, image.cols);

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
      printf("Depth blur with focus at %d, %d\n", pointx, pointy);
      depth_blur(&image, &depth, pointx, pointy, &display);
      printf("Depth filter done!\n");
      cv::imshow("Blur window", display);
      break;
    case KEY_ENTER:
      printf("Bilateral filter\n");
      bilateral_filter(&image, &display);
      //cv::bilateralFilter(image, display, 13, 30, 30);
      printf("Bilateral filter done!\n");
      cv::imshow("Blur window", display);
      break;
    case KEY_S:
        printf("Saving image in res.png\n");
        cv::imwrite("res.png", display);
      break;
    case KEY_DOWN:
      if(pointy >= display.rows)
        break;
      pointy = (pointy + TARGET_STEP);
      draw_target(disp_src, pointx, pointy, &display);
      break;
    case KEY_UP:
      if(pointy <= 0)
        break;
      pointy = (pointy - TARGET_STEP);
      draw_target(disp_src, pointx, pointy, &display);
      break;
    case KEY_LEFT:
      if(pointx <= 0)
        break;
      pointx = (pointx - TARGET_STEP);
      draw_target(disp_src, pointx, pointy, &display);
      break;
    case KEY_RIGHT:
      if(pointx >= display.cols)
        break;
      pointx = (pointx + TARGET_STEP);
      draw_target(disp_src, pointx, pointy, &display);
      break;
    case KEY_ALT:
      if(!display_depth)
      {
        cv::imshow("Blur window", depth);
        display_depth = true;
        disp_src = &depth;
      }
      else
      {
        cv::imshow("Blur window", display);
        display_depth = false;
        disp_src = &image;
      }
      draw_target(disp_src, pointx, pointy, &display);
      break;
    default:
      printf("key = %d\n", key);
      break;
    }
  }
  return 0;
}
