#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace std;

int get_sequence(const cv::Mat * img, int n, cv::Mat ** seq)
{

    cv::Mat img_expanded;

    cv::copyMakeBorder(*img, img_expanded, 200, 200, 200, 200, cv::BORDER_CONSTANT);

    for(int i = 0; i < n; i++)
    {
        double m[2][3] = {{1.0, 0.0, i * 8.0}, {0.0, 1.0, i * 4.0}};
        cv::Mat M = cv::Mat(2, 3, CV_64F, m);
        seq[i] = new cv::Mat(img_expanded.size(), img_expanded.type());
        cv::warpAffine(img_expanded, *seq[i], M, img_expanded.size());
    }

    return 0;
}

//dst is passed via pointer and allocated within this
//function because I dont know the exact size of dst
//matrix until I call get_sequence()
int get_blured(const cv::Mat * img, int n, cv::Mat * &dst)
{
    cv::Mat ** seq = new cv::Mat*[n];
    get_sequence(img, n, seq);

    dst = new cv::Mat(seq[0]->size(), seq[0]->type());

    seq[0]->copyTo(*dst);

    for(int i = 0; i < n; i++)
    {
        cv::addWeighted(*dst, 0.5, *seq[i], 0.5, 0, *dst);
    }

    for(int i = 0; i < n; i++)
    {
        delete seq[i];
    }

    delete seq;

    return 0;
}

int compose(const cv::Mat * fg, const cv::Mat * bg, cv::Mat * dst)
{

}

int main(int argc, char ** argv)
{
    int n = 10;

    cv::Mat img = cv::imread("smallball.png");
    cv::Mat * dst = NULL;
    get_blured(&img, n, dst);

    cv::imwrite("res.png", *dst);

    delete dst;

    return 0;
}