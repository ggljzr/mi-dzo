#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#define NOT_8UC3_IMAGE -3
#define NOT_MATCHING_TYPE -2
#define NOT_MATCHING_SIZE -1

using namespace std;

void print_matrix(const cv::Mat * mat)
{
    printf("Size: %d r %d c\n", mat->rows, mat->cols);
    printf("Type: %d, channels: %d\n", mat->type(), mat->channels());

    int rows = mat->rows;
    int channels = mat->channels();
    int cols = mat->cols * channels;

    for(int i = 0; i < rows; i++)
    {
        const uchar * row = mat->ptr<uchar>(i);
        for(int j = 0; j < cols; j += channels )
        {
            printf("[ ");
            for(int k = 0; k < channels; k++)
            {
                printf("c%d=%d, ", k, row[j + k]);
            }
            printf(" ], ");
        }
        printf("\n");
    }
}

cv::Mat * get_neighbours(const cv::Mat * mat, int x, int y, int size)
{
    cv::Rect r(y-size/2,x-size/2,size, size);

    cv::Mat * res = NULL;

    if(0 <= r.x && 0 <= r.width && 
    	r.x + r.width <= mat->cols &&
    	0 <= r.y && 0 <= r.height &&
    	r.y + r.height <= mat->rows)
    {
    	res = new cv::Mat(*mat, r);
    }
    else
    {
    	res = new cv::Mat(size, size, CV_8UC3, cvScalar(0));
    }

    return res;
}


/*
color:
0 - r
1 - g
2 - b
*/
int get_average(const cv::Mat * mat, char color)
{
	if(mat->type() != CV_8UC3)
		return NOT_8UC3_IMAGE;

	int rows = mat->rows;
	int channels = mat->channels();
	int cols = mat->cols * channels;

	int sum = 0;

	for(int i = 0; i < rows; i++)
	{
		const uchar * row = mat->ptr<uchar>(i);
		for(int j = 0; j < cols; j += channels)
		{
			sum += row[j + color];
		}
	}

	sum = sum / (rows * mat->cols);
	return sum;
}

int depth_blur(const cv::Mat * mat, const cv::Mat * depth, cv::Mat * res)
{
    if(mat->rows != res->rows || mat->cols != res->cols)
        return NOT_MATCHING_SIZE;
    if(mat->type() != res->type())
    	return NOT_MATCHING_TYPE;
    if(mat->type() != CV_8UC3)
    	return NOT_8UC3_IMAGE;


    int rows = mat->rows;
    int channels = mat->channels();
    int cols = mat->cols * channels;

    for(int i = 1; i < rows; i++)
    {
    	const uchar * row_in = mat->ptr<uchar>(i);
        const uchar * row_depth = depth->ptr<uchar>(i);
    	uchar * row_out = res->ptr<uchar>(i);
    	for(int j = 3; j < cols; j += channels)
    	{
            int k = row_depth[j];
            if(k % 2 == 0)
                k++;

    		cv::Mat * neigbours = get_neighbours(mat, i, j / channels, k);

    		row_out[j] = get_average(neigbours, 0);
    		row_out[j + 1] = get_average(neigbours, 1);
    		row_out[j + 2] = get_average(neigbours, 2);

    		delete neigbours;
    	}
    }

    return 0;
}

int main( int argc, char** argv )
{

    if(argc < 3)
    {
        cout << "Usage: ./DepthBlur image.png depth.png" << endl;
        return 0;
    }

	cv::Mat image = cv::imread(argv[1]);
	cv::Mat depth = cv::imread(argv[2]);

    double min, max;
    cv::minMaxLoc(depth, &min, &max);
    cout << min << endl;
    cout << max << endl;

    cv::Mat B = cv::Mat::zeros(image.rows, image.cols, CV_8UC3);
    
    depth_blur(&image, &depth, &B);

    //cv::imshow("img", image);
    cv::imwrite("res.png", B);

    cv::waitKey(0);                                          // Wait for a keystroke in the window
    return 0;
}
