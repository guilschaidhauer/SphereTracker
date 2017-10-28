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

//Points
int points = 0;
vector<Circle> activeCircles;
time_t lastGenerationTime, currentTime;
int minx = 50;
int maxx = 600;
float minSpeed = 4;
float maxSpeed = 8;
bool gameOn = true;
int toBeGenerated = 2;

void generateCircle()
{
	Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	activeCircles.push_back(Circle(Point2f(generation.uniform(minx, maxx), -generation.uniform(0, 20)), 10, generation.uniform(minSpeed, maxSpeed), color));
}

void initCircles()
{
	time(&lastGenerationTime);
	//for (int i = 0; i < 3; i++)
	//{
		generateCircle();
	//}
}

Mat drawCircles(Mat toDrawImage)
{
	for (int i = 0; i < activeCircles.size(); i++)
	{
		circle(toDrawImage, activeCircles[i].Center, activeCircles[i].Radius, activeCircles[i].Color, 2, 8, 0);
	}

	return toDrawImage;
}

void generateCircles()
{
	time(&currentTime);
	// Time elapsed
	double seconds = difftime(currentTime, lastGenerationTime);
	if (seconds > 1)
	{
		for (int i = 0; i < toBeGenerated; i++)
		{
			generateCircle();
		}
		time(&lastGenerationTime);
	}
}

void moveCircles()
{
	for (int i = 0; i < activeCircles.size(); i++)
	{
		activeCircles[i].Center.y += activeCircles[i].Speed;

		if (activeCircles[i].Center.y > 500)
		{
			activeCircles.erase(activeCircles.begin() + i);
			cout << "Points: " << points << endl;
			gameOn = false;
		}
	}
}

bool checkCollision(Circle x, Circle y)
{
	if (sqrt((y.Center.x - x.Center.x) * (y.Center.x - x.Center.x) + (y.Center.y - x.Center.y) * (y.Center.y - x.Center.y)) < (x.Radius + y.Radius))
	{
		return true;
	}

	return false;
}

void checkAndHandleCollisions(Circle mainCircle)
{
	vector<int> indexes;
	for (int i = 0; i < activeCircles.size(); i++)
	{
		if (checkCollision(mainCircle, activeCircles[i]))
		{
			indexes.push_back(i);
		}
	}

	for (int i = 0; i < indexes.size(); i++)
	{
		activeCircles.erase(activeCircles.begin() + indexes[i]);
		points++;
	}

	if (points > 5)
	{
		toBeGenerated = 4;
	}
	else if (points > 20)
	{
		toBeGenerated = 5;
		minSpeed = 8;
		maxSpeed = 12;
	}
	else if (points > 50)
	{
		toBeGenerated = 7;
	}
	else if (points > 100)
	{
		minSpeed = 10;
		maxSpeed = 14;
	}

	//cout << points << endl;
}

int main(int argc, char** argv)
{
	red = Scalar(0, 0, 255);
	VideoCapture cap(0); //capture the video from webcam

	if (!cap.isOpened())  // if not success, exit program
	{
		cout << "Cannot open the web cam" << endl;
		return -1;
	}

	namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

	int iLowH = 18;
	int iHighH = 119;

	int iLowS = 47;
	int iHighS = 255;
	 
	int iLowV = 154;
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

	initCircles();

	while (gameOn)
	{
		Mat imgOriginal;

		bool bSuccess = cap.read(imgOriginal); // read a new frame from video


		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

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
			if (contours[i].size() > 100)
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
				checkAndHandleCollisions(Circle(center[i], (int)radius[i] / 2, 99, color));
			}
		}

		drawing = drawCircles(drawing);
		//moveCircles();
		generateCircles();

		cv::flip(drawing, drawing, 1);
		namedWindow("Contours", WINDOW_AUTOSIZE);
		imshow("Contours", drawing);

		cv::flip(imgOriginal, imgOriginal, 1);
		imshow("Original", imgOriginal);

		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}

	system("pause");
	return 0;
}