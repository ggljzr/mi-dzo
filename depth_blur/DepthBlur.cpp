#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define NOT_8UC3_IMAGE -3
#define NOT_MATCHING_TYPE -2
#define NOT_MATCHING_SIZE -1

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

void depth_visualise(const cv::Mat* depth) {
  double min;
  double max;
  cv::minMaxIdx(*depth, &min, &max);
  cv::Mat adjMap;
  cv::convertScaleAbs(*depth, adjMap, 255 / max);
  cv::imwrite("depth_vis.png", adjMap);
}

int depth_blur(const cv::Mat* mat, const cv::Mat* depth, cv::Mat* res) {
  if (mat->rows != res->rows || mat->cols != res->cols)
    return NOT_MATCHING_SIZE;
  if (mat->type() != res->type()) return NOT_MATCHING_TYPE;
  if (mat->type() != CV_8UC3) return NOT_8UC3_IMAGE;

  int rows = mat->rows;
  int channels = mat->channels();
  int cols = mat->cols * channels;

  for (int i = 1; i < rows; i++) {
    const uchar* row_in = mat->ptr<uchar>(i);
    const uchar* row_depth = depth->ptr<uchar>(i);
    uchar* row_out = res->ptr<uchar>(i);
    for (int j = 3; j < cols; j += channels) {
      int k = get_kernel(row_depth[j]);
      cv::Mat* neigbours = get_neighbours(mat, i, j / channels, k);

      row_out[j] = get_average(neigbours, 0);
      row_out[j + 1] = get_average(neigbours, 1);
      row_out[j + 2] = get_average(neigbours, 2);

      delete neigbours;
    }
  }

  return 0;
}

int main(int argc, char** argv) {
  if (argc < 3) {
    cout << "Usage: ./DepthBlur image.png depth.png" << endl;
    return 0;
  }

  cv::Mat image = cv::imread(argv[1]);
  cv::Mat depth = cv::imread(argv[2]);

  cv::Mat blur = cv::Mat::zeros(image.rows, image.cols, CV_8UC3);

  depth_blur(&image, &depth, &blur);

  // cv::imshow("img", image);
  cv::imwrite("blur.png", blur);

  return 0;
}
