#include <iostream>
#include <string>
#include <opencv2/videoio.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio/legacy/constants_c.h>

using namespace cv;
using namespace std;

void displayAsImage(Mat img)
{
    namedWindow("Display", WINDOW_AUTOSIZE);
    imshow("Img", img);
    waitKey(0);
}

Mat processFrame(Mat frame, Mat background)
{
    Mat result;

    flip(frame, frame, 1);

    // opencv read imgs as BGR, convert to hsv which is more similar to how humans perceive colour and will allow
    // for dectection of a range of shades of the same colour
    Mat hsv_frame;
    cvtColor(frame, hsv_frame, COLOR_BGR2HSV);

    // create mask for cloak
    Mat cloak_mask;
    inRange(hsv_frame, Scalar(40, 40, 40), Scalar(170, 255, 255), cloak_mask); // (hue, saturation, value/brightness)

    // clean cloak_mask
    Mat kernel3x3 = Mat::ones(3, 3, CV_32F);
    Mat kernel5x5 = Mat::ones(5, 5, CV_32F);
    Mat kernel7x7 = Mat::ones(7, 7, CV_32F);
    Mat kernel9x9 = Mat::ones(9, 9, CV_32F);
    Mat kernel50x50 = Mat::ones(50, 50, CV_32F);
    morphologyEx(cloak_mask, cloak_mask, MORPH_OPEN, kernel9x9);   //erosion (removes noise) + dilation (expands)
    morphologyEx(cloak_mask, cloak_mask, MORPH_DILATE, kernel9x9); //second dilation

    // create surrounding mask (invert cloak_mask)
    Mat surrounding_mask;
    bitwise_not(cloak_mask, surrounding_mask);

    // segment out cloak
    Mat segmented_frame;
    bitwise_and(frame, frame, segmented_frame, surrounding_mask);

    // create hole fill
    Mat background_fill;
    bitwise_and(background, background, background_fill, cloak_mask);

    // create result
    addWeighted(segmented_frame, 1, background_fill, 1, 0, result);

    return result;
}

int main()
{
    VideoCapture cap("video.mp4");
    if (!cap.isOpened())
    {
        cout << "Error opening file" << endl;
        return -1;
    }

    int frame_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    VideoWriter video("result.mp4", CV_FOURCC('m','p','4','v'), 20, Size(frame_width,frame_height));

    // assuming first couple frames are of background, get background
    Mat background, frame;
    cap >> frame;
    for (int i = 0; i < 30; i++)
    {
        cap >> background;
    }
    flip(background, background, 1);

    Mat processedFrame;
    while (!frame.empty())
    {
        processedFrame = processFrame(frame, background);
        video.write(processedFrame);
        cap >> frame;
    }

    cap.release();
    video.release();

    return 0;
}