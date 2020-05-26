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

  // assume first couple frames are of background
  Mat background;
  for (int i = 0; i < 30; i++)
  {
    cap >> background;
  }
  flip(background, background, 1);

  // get frame
  Mat frame;
  for (int x = 0; x < 139; x++)
    cap >> frame;
  flip(frame, frame, 1);

  // opencv read imgs as BGR, convert to hsv which is more similar to how humans perceive colour and will allow
  // for dectection of a range of shades of the same colour
  Mat hsv_frame;
  cvtColor(frame, hsv_frame, COLOR_BGR2HSV);

  // create mask for cloak
  Mat cloak_mask;
  inRange(hsv_frame, Scalar(118, 10, 70), Scalar(175, 255, 255), cloak_mask); // (hue, saturation, value/brightness)

  imshow("frame", frame);
  imshow("hsv_frame", hsv_frame);
  imshow("cloak_mask", cloak_mask);
  waitKey(0);
  destroyAllWindows();

  // clean cloak_mask
  Mat kernel3x3 = Mat::ones(3, 3, CV_32F);
  Mat kernel5x5 = Mat::ones(5, 5, CV_32F);
  Mat kernel7x7 = Mat::ones(7, 7, CV_32F);
  Mat kernel9x9 = Mat::ones(9, 9, CV_32F);

  Mat ellipse5x5 = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
  Mat ellipse7x7 = getStructuringElement(MORPH_ELLIPSE, Size(7, 7));

  morphologyEx(cloak_mask, cloak_mask, MORPH_ERODE, kernel5x5);
  morphologyEx(cloak_mask, cloak_mask, MORPH_OPEN, kernel5x5);

  imshow("clean cloak_mask", cloak_mask);
  waitKey(0);
  
  // find remaining noise and mask
  Mat contourMask = Mat::zeros(frame_height, frame_width, CV_8U);
  vector<Mat> contours;
  findContours(cloak_mask, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

  vector<vector<Point> > contours_poly(contours.size());
  vector<Point2f>centers(contours.size());
  vector<float>radius(contours.size());
  for(int i = 0; i < contours.size(); i++)
  {
    auto c = contours[i];
    int count = 0;
    double arc = arcLength(c, true);

    if(arc < 56)
    {
      approxPolyDP( contours[i], contours_poly[i], 1, true );
      minEnclosingCircle( contours_poly[i], centers[i], radius[i] );
      drawContours( contourMask, contours_poly, (int)i, 255 );
      circle( contourMask, centers[i], (int)radius[i], 255, -1 );
    }
  }
  morphologyEx(contourMask, contourMask, MORPH_DILATE, kernel3x3, Point(-1,-1), 1);

  imshow("contourMask", contourMask);
  waitKey(0);

  // create surrounding mask (invert cloak_mask)
  Mat surrounding_mask;
  bitwise_not(cloak_mask, surrounding_mask);
  addWeighted(surrounding_mask, 1, contourMask, 1, 0, surrounding_mask);
  morphologyEx(surrounding_mask, surrounding_mask, MORPH_ERODE, ellipse7x7, Point(-1,-1), 1); // does opposite of erode in this case

  imshow("surrounding_mask", surrounding_mask);
  waitKey(0);
  destroyAllWindows();

  // segment out cloak
  Mat segmented_frame;
  bitwise_and(frame, frame, segmented_frame, surrounding_mask);
  imshow("segmented_frame", segmented_frame);
  waitKey(0);

  // overlay background on cloak hole
  Mat background_fill;
  subtract(cloak_mask, contourMask, cloak_mask);
  morphologyEx(cloak_mask, cloak_mask, MORPH_DILATE, ellipse7x7, Point(-1,-1), 1);
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