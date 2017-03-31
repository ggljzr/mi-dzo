#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace std;

#define NOT_SAME_TYPE -1

int get_sequence(const cv::Mat * img, int n, cv::Mat ** seq, double x, double y)
{

    cv::Mat img_expanded;
    int rows = img->rows;
    int cols = img->cols;

    cv::copyMakeBorder(*img, img_expanded, 0, n*y, 0, n*x, cv::BORDER_CONSTANT);

    for(int i = 0; i < n; i++)
    {
        double m[2][3] = {{1.0, 0.0, i * x}, {0.0, 1.0, i * y}};
        cv::Mat M = cv::Mat(2, 3, CV_64F, m);
        seq[i] = new cv::Mat(img_expanded.size(), img_expanded.type());
        cv::warpAffine(img_expanded, *seq[i], M, img_expanded.size());
    }

    return 0;
}

//dst is passed via pointer and allocated within this
//function because I dont know the exact size of dst
//matrix until I call get_sequence()
int get_blured(const cv::Mat * img, int n, cv::Mat * &dst,
                double x = 8.0, double y = 4.0)
{
    cv::Mat ** seq = new cv::Mat*[n];
    get_sequence(img, n, seq, x, y);

    dst = new cv::Mat(seq[0]->size(), seq[0]->type());

    seq[0]->copyTo(*dst);

    for(int i = 0; i < n; i++)
    {
        cv::addWeighted(*dst, 0.6, *seq[i], 0.4, 0, *dst);
    }

    for(int i = 0; i < n; i++)
    {
        delete seq[i];
    }

    delete seq;

    return 0;
}

int compose(const cv::Mat * fg, cv::Mat * bg, int x, int y)
{
    if(fg->channels() != 3)
        cv::cvtColor(*fg, *fg, CV_GRAY2RGB);
    
    if(bg->channels() != 3)
        cv::cvtColor(*bg, *bg, CV_GRAY2RGB);

    int rows = fg->rows;
    int cols = fg->cols * fg->channels();

    for(int i = 0; i < rows; i++)
    {
        uchar * row_bg = bg->ptr<uchar>(i + y);
        const uchar * row_fg = fg->ptr<uchar>(i); 
        for(int j = 0; j < cols; j += 3)
        {
            int r = row_fg[j];
            int g = row_fg[j + 1];
            int b = row_fg[j + 2];

            if(r > 10 && g > 10 && b > 10)
            {

            row_bg[j + x] = r;
            row_bg[j + x + 1] = g;
            row_bg[j + x + 2] = b;}
        }
    }
}   

int main(int argc, char ** argv)
{
    int n = 20;

    if(argc < 2)
    {
        cout << "usage: ./MotionBlur image.png" << endl;
    }

    cv::Mat img = cv::imread(argv[1]);
    //cv::Mat bg = cv::imread("bg.png");

    //cout << img.type() << endl;

    cv::Mat * dst = NULL;
    get_blured(&img, n, dst, 16, 8);
    
    //compose(dst, &bg, 0, 0);

    cv::imwrite("res.png", *dst);
    cv::imwrite("res.png", img);

    delete dst;

    return 0;
}