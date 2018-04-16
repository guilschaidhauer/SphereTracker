#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>
#include <time.h>
#include <math.h>

using namespace cv;
using namespace std;

Scalar red;
float highestRadius;
RNG rng(12345);
RNG generation(time(NULL));

struct Circle
{
	Circle(Point2f center, int radius, float speed, Scalar color) : Center(center), Radius(radius), Speed(speed), Color(color) {}
	int Radius;
	Point2f Center;
	float Speed;
	Scalar Color;
};

int detectAndDrawCircle(VideoCapture cap, int iLowH, int iHighH, int iLowS, int iHighS, int iLowV, int iHighV, Scalar red, String windowName, String tWindowsName)
{
	int returnValue = 0;
	Mat imgOriginal1;

	bool bSuccess_1 = cap.read(imgOriginal1); // read a new frame from video

	if (!bSuccess_1) //if not success, break loop
	{
		cout << "Cannot read a frame from video stream" << endl;
		return 0;
	}

	cv::flip(imgOriginal1, imgOriginal1, 1);
	//imshow("Original", imgOriginal);

	Mat imgHSV_1;
	cvtColor(imgOriginal1, imgHSV_1, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

	Mat imgThresholded_1;
	inRange(imgHSV_1, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded_1); //Threshold the image //morphological opening (removes small objects from the foreground)

	erode(imgThresholded_1, imgThresholded_1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(imgThresholded_1, imgThresholded_1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	//morphological closing (removes small holes from the foreground)
	dilate(imgThresholded_1, imgThresholded_1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(imgThresholded_1, imgThresholded_1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	//imshow(tWindowsName, imgThresholded_1); //show the thresholded image

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(imgThresholded_1, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

	vector<vector<Point> > contours_poly(contours.size());
	vector<Rect> boundRect(contours.size());
	vector<Point2f>center(contours.size());
	vector<float>radius(contours.size());

	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > 5)
		{
			approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
			boundRect[i] = boundingRect(Mat(contours_poly[i]));
			minEnclosingCircle((Mat)contours_poly[i], center[i], radius[i]);
		}
	}

	/// Draw polygonal contour + bonding rects + circles
	Mat drawing = Mat::zeros(imgThresholded_1.size(), CV_8UC3);
	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > 100)
		{
			circle(imgOriginal1, center[i], (int)radius[i], red, 4, 8, 0);
			circle(imgOriginal1, center[i], 5, red, -1);

			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			circle(drawing, center[i], 20, color, 4, 8, 0);
			returnValue = 1;
		}
	}

	imshow(windowName, imgOriginal1);

	return returnValue;
}

int main(int argc, char** argv)
{
	red = Scalar(0, 0, 255);

	string filename = "multicolor.mp4";

	VideoCapture cap(filename); //capture the video from webcam
	//VideoCapture cap(0); //capture the video from webcam

	if (!cap.isOpened())  // if not success, exit program
	{
		cout << "Cannot open the web cam" << endl;
		system("pause");
		return -1;
	}

	namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

	//Yellow
	int iLowH_1 = 23;
	int iHighH_1 = 38;

	int iLowS_1 = 52;
	int iHighS_1 = 255;

	int iLowV_1 = 229;
	int iHighV_1 = 255;

	//Green
	int iLowH_2 = 37;
	int iHighH_2 = 88;

	int iLowS_2 = 47;
	int iHighS_2 = 255;

	int iLowV_2 = 232;
	int iHighV_2 = 255;

	//Create trackbars in "Control" window
	createTrackbar("LowH", "Control", &iLowH_1, 179); //Hue (0 - 179)
	createTrackbar("HighH", "Control", &iHighH_1, 179);

	createTrackbar("LowS", "Control", &iLowS_1, 255); //Saturation (0 - 255)
	createTrackbar("HighS", "Control", &iHighS_1, 255);

	createTrackbar("LowV", "Control", &iLowV_1, 255);//Value (0 - 255)
	createTrackbar("HighV", "Control", &iHighV_1, 255);

	int iLastX = -1;
	int iLastY = -1;

	//Capture a temporary image from the camera
	Mat imgTmp;
	cap.read(imgTmp);

	while (true)
	{
		if (detectAndDrawCircle(cap, iLowH_1, iHighH_1, iLowS_1, iHighS_1, iLowV_1, iHighV_1, red, "Circle 1", "Threshold window 1") == 1)
		{
			cout << "On" << endl;
		}
		else 
		{
			cout << "Off" << endl;
		}
		//detectAndDrawCircle(cap, iLowH_2, iHighH_2, iLowS_2, iHighS_2, iLowV_2, iHighV_2, red, "Circle 2", "Threshold window 2");

		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}

	return 0;
}