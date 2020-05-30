/**
 * 
 * Takes webcam feed as input, segments out the "cloak", and overlays the 
 * background on top of it creating an invisibility cloak effect
 * 
 **/

#include <iostream>
#include <string>
#include <opencv2/videoio.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio/legacy/constants_c.h>

using namespace cv;
using namespace std;

Mat processFrame(Mat frame, Mat background, int frame_width, int frame_height)
{
    Mat result;
    flip(frame, frame, 1);

    // opencv read imgs as BGR, convert to hsv which is more similar to how humans perceive colour and will allow
    // for dectection of a range of shades of the same colour
    Mat hsv_frame;
    cvtColor(frame, hsv_frame, COLOR_BGR2HSV);

    // create mask for cloak by detecting cloth colour
    Mat cloak_mask;
    inRange(hsv_frame, Scalar(118, 10, 70), Scalar(175, 255, 255), cloak_mask); // (hue, saturation, value/brightness)

    // some kernels to be used during segmentation process
    Mat kernel3x3 = Mat::ones(3, 3, CV_32F);
    Mat kernel5x5 = Mat::ones(5, 5, CV_32F);
    Mat ellipse7x7 = getStructuringElement(MORPH_ELLIPSE, Size(7, 7));

    // clean cloak_mask a bit, erosion follwed by open operation to remove some noise and expand eroded mask
    morphologyEx(cloak_mask, cloak_mask, MORPH_ERODE, kernel5x5);
    morphologyEx(cloak_mask, cloak_mask, MORPH_OPEN, kernel5x5);

    // find remaining noise and generate mask for this noise
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

    // create mask for surrounding (invert cloak_mask)
    Mat surrounding_mask;
    bitwise_not(cloak_mask, surrounding_mask);
    addWeighted(surrounding_mask, 1, contourMask, 1, 0, surrounding_mask);
    morphologyEx(surrounding_mask, surrounding_mask, MORPH_ERODE, ellipse7x7, Point(-1,-1), 1); //does opposite of erode in this case

    // segment out cloak
    Mat segmented_frame;
    bitwise_and(frame, frame, segmented_frame, surrounding_mask);

    // overlay background on cloak hole
    Mat background_fill;
    subtract(cloak_mask, contourMask, cloak_mask);
    morphologyEx(cloak_mask, cloak_mask, MORPH_DILATE, ellipse7x7, Point(-1,-1), 1);
    bitwise_and(background, background, background_fill, cloak_mask);

    // create result
    addWeighted(segmented_frame, 1, background_fill, 1, 0, result);

    return result;
}


int main()
{
    // open webcam and read
    VideoCapture cap;
    cap.open(0);
    if (!cap.isOpened())
    {
        cout << "Unable to open camera" << endl;
        return -1;
    }
    int frame_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);


    Mat background, frame, processedFrame;

    // assume first couple frames are of background
    for (int x = 0; x < 50; x++)
    {
        cap >> background;
        if (background.empty())
            cout << "Couldn't get background!" << endl;
    }

    while (1)
    {
        // get frame
        cap >> frame;
        if (frame.empty())
        {
            cout << "Empty frame!" << endl;
            break;
        }
        flip(frame, frame, 1);

        // apply invisibility cloak effect
        processedFrame = processFrame(frame, background, frame_width, frame_height);

        // show for long enough that you can actually see it, still looks like a live webcam
        imshow("processedFrame", processedFrame);
        if (waitKey(5) >= 0)
            break;
    }

    destroyAllWindows();
    cap.release();

    return 0;
}