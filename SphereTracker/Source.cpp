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

	//Green
	//int iLowH = 37;
	//int iHighH = 88;

	//int iLowS = 47;
	//int iHighS = 255;

	//int iLowV = 232;
	//int iHighV = 255;

	//Yellow
	int iLowH = 23;
	int iHighH = 38;

	int iLowS = 52;
	int iHighS = 255;

	int iLowV = 229;
	int iHighV = 255;

	//Create trackbars in "Control" window
	createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	createTrackbar("HighH", "Control", &iHighH, 179);

	createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	createTrackbar("HighS", "Control", &iHighS, 255);

	createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
	createTrackbar("HighV", "Control", &iHighV, 255);

	int iLastX = -1;
	int iLastY = -1;

	//Capture a temporary image from the camera
	Mat imgTmp;
	cap.read(imgTmp);

	//Create a black image with the size as the camera output
	Mat imgLines = Mat::zeros(imgTmp.size(), CV_8UC3);;

	while (true)
	{
		Mat imgOriginal;

		bool bSuccess = cap.read(imgOriginal); // read a new frame from video


		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

		cv::flip(imgOriginal, imgOriginal, 1);
		//imshow("Original", imgOriginal);

		Mat imgHSV;
		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

		Mat imgThresholded;
		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image //morphological opening (removes small objects from the foreground)
		
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		//morphological closing (removes small holes from the foreground)
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		//Calculate the moments of the thresholded image
		Moments oMoments = moments(imgThresholded);

		double dM01 = oMoments.m01;
		double dM10 = oMoments.m10;
		double dArea = oMoments.m00;

		imshow("Thresholded Image", imgThresholded); //show the thresholded image

		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		findContours(imgThresholded, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

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
		Mat drawing = Mat::zeros(imgThresholded.size(), CV_8UC3);
		for (int i = 0; i< contours.size(); i++)
		{
			if (contours[i].size() > 100)
			{
				circle(imgOriginal, center[i], (int)radius[i], red, 4, 8, 0);
				circle(imgOriginal, center[i], 5, red, -1);

				Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
				circle(drawing, center[i], 20, color, 4, 8, 0);
			} 
		}

		//drawing = drawCircles(drawing);
		//moveCircles();
		//generateCircles();

		//cv::flip(drawing, drawing, 1);
		//namedWindow("Contours", WINDOW_AUTOSIZE);
		//imshow("Contours", drawing);

		//cv::flip(imgOriginal, imgOriginal, 1);
		imshow("Circle", imgOriginal);

		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}
	return 0;
}