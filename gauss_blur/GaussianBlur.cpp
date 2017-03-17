#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace std;

cv::Mat * get_neighbours(const cv::Mat * mat, int x, int y, int size)
{
    cv::Rect r(y-size/2,x-size/2,size, size);
    cv::Mat * res = new cv::Mat(*mat, r);

    return res;
}

int main( int argc, char** argv )
{
    cv::Mat image, depth;
    image = cv::imread("desk_3_1.png");   // Read the file
    depth = cv::imread("desk_3_1_depth.png");

    cv::imshow("aaa", image);

    cv::waitKey();

    return 0;
}