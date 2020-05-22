// Example program
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


  // assuming first couple frames are of background, get background
  Mat background;
  for (int i = 0; i < 30; i++)
  {
    cap >> background;
  }
  flip(background, background, 1);

  Mat frame;
  for (int x = 0; x < 450; x++)
    cap >> frame;
  flip(frame, frame, 1);

  // opencv read imgs as BGR, convert to hsv which is more similar to how humans perceive colour and will allow
  // for dectection of a range of shades of the same colour
  Mat hsv_frame;
  cvtColor(frame, hsv_frame, COLOR_BGR2HSV);

  // create mask for cloak
  Mat cloak_mask;
  inRange(hsv_frame, Scalar(40, 40, 40), Scalar(170, 255, 255), cloak_mask); // (hue, saturation, value/brightness)

  imshow("frame", frame);
  imshow("hsv_frame", hsv_frame);
  imshow("cloak_mask", cloak_mask);
  waitKey(0);

  destroyAllWindows();

  // clean cloak_mask
  Mat kernel3x3 = Mat::ones(3, 3, CV_32F);
  Mat kernel5x5 = Mat::ones(5, 5, CV_32F);
  Mat kernel7x7 = Mat::ones(7, 7, CV_32F);
  Mat kernel50x50 = Mat::ones(50, 50, CV_32F);
  morphologyEx(cloak_mask, cloak_mask, MORPH_OPEN, kernel50x50);   //erosion (removes noise) + dilation (expands)
  morphologyEx(cloak_mask, cloak_mask, MORPH_DILATE, kernel50x50); //second dilation
  //  morphologyEx(cloak_mask, cloak_mask, MORPH_OPEN, kernel9x9);

  // morphologyEx(cloak_mask, cloak_mask, MORPH_ERODE, kernel);
  imshow("clean cloak_mask", cloak_mask);
  waitKey(0);

  // Mat edge;
  // vector<Mat> contours;
  // Canny(cloak_mask, edge, 50, 100);
  // imshow("edge", edge);
  // waitKey(0);

  // Mat contourMask = Mat::ones(frame_height, frame_width, CV_8U);
  // findContours(edge, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
  // for(auto &c : contours)
  // {
  //   int count = 0;
  //   double arc = arcLength(c, true);
  //   double area = contourArea(c);
  //   cout << arc << " " << contourArea(c) << endl;
  //   if(area < 1000.0)
  //   {
  //     drawContours(contourMask, c, -1, 0, -1); //CV_FILLED = -1
  //     // fillPoly(contourMask, c, Scalar(0,0,0));
  //   }
  //   imshow("contourMask", contourMask);
  //   waitKey(0);
  // }

  // create surrounding mask (invert cloak_mask)
  Mat surrounding_mask;
  bitwise_not(cloak_mask, surrounding_mask);
  imshow("surrounding_mask", surrounding_mask);
  waitKey(0);

  destroyAllWindows();

  // segment out cloak
  Mat segmented_frame;
  bitwise_and(frame, frame, segmented_frame, surrounding_mask);
  imshow("segmented_frame", segmented_frame);
  waitKey(0);

  // create hole fill
  Mat background_fill;
  bitwise_and(background, background, background_fill, cloak_mask);
  imshow("background_fill", background_fill);
  waitKey(0);

  destroyAllWindows();

  // final result
  Mat result;
  addWeighted(segmented_frame, 1, background_fill, 1, 0, result);
  imshow("result", result);
  waitKey(0);

  return 0;
}