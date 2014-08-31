#include <Windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <ctime>

using namespace cv;
using namespace std;

/** DEMO **/
bool arrowKeyDemo = false;
/** DEMO **/

//constants
#define ESC_KEY 27
#define PI 3.14159265

//global variables
int erosion_elem = 0;
int erosion_size = 3;
int dilation_elem = 0;
int dilation_size = 4;
int const max_elem = 2;
int const max_kernel_size = 21;
Scalar avgSkinCol[6];
int numSamples = 6;

//Windows input
INPUT ip[2];
bool leftPressed = false;
bool rightPressed = false;

//create elements for morphological tranforms
Mat eroElement = getStructuringElement(MORPH_RECT,
	Size(2 * erosion_size + 1, 2 * erosion_size + 1),
	Point(erosion_size, erosion_size));
Mat dilElement = getStructuringElement(MORPH_RECT,
	Size(2 * dilation_size + 1, 2 * dilation_size + 1),
	Point(dilation_size, dilation_size));

//function headers
Mat filterHand(VideoCapture cap);
Scalar detectSkinColor(Mat frame, Rect scanBox);
Mat scanBox(Rect box, Mat frame, int boxNum);
void detectHand(Mat frame);
vector<Vec4i> findFingerDefects(vector<Vec4i> defects, vector<Point> contours);
int clamp(int n);
void leftPress();
void rightPress();
void spacePress();
void upRightDiag();
void upLeftDiag();
void releaseKey();
void initKeyboard();
void checkFingers();
float findAngle(Point start, Point end, Point depth);

int main(){
	Mat frame;

	// open the default camera
	VideoCapture cap(0);

	//fix exposure
	cap.set(CV_CAP_PROP_SETTINGS, 1);

	// check if we succeeded
	if (!cap.isOpened())
		return -1;

	if (arrowKeyDemo)
		initKeyboard();

	frame = filterHand(cap);

	detectHand(frame);
	//HSVMethod();

	waitKey(0);

	return 0;
}

void detectHand(Mat frame){

}

Mat filterHand(VideoCapture cap){
	//performance measurements
	clock_t begin, end;
	double tGaussBlr, tThreshold, tCombine, tMedBlur, tMorph, tDisplayResults;
	Point mousePos(0,0);

	Mat finFrame;

	while (1){
		Mat frame, finImage, showFrame, channel[3], threshImage[6];

		//get frame from camera
		cap >> frame;
		flip(frame, frame, 1);

		//make a copy
		frame.copyTo(showFrame);

		//soften image
		begin = clock();
		//GaussianBlur(frame, frame, Size(5, 5), 0);
		tGaussBlr = double(clock() - begin) / CLOCKS_PER_SEC;

		cvtColor(frame, frame, CV_BGR2HSV);

		//Create ROIs
		int square = 25;
		Rect box1(frame.cols / 2 - square / 2, frame.rows / 2 - (5 * square / 2), square, square); //up
		Rect box2(frame.cols / 2 - square / 2, frame.rows / 2 - square / 2, square, square); //mid
		Rect box3(frame.cols / 2 - 2 * square, frame.rows / 2 - (square / 2), square, square); //left mid
		Rect box4(frame.cols / 2 - 2 * square, frame.rows / 2 + (3 * square / 2), square, square); //left low
		Rect box5(frame.cols / 2 + square, frame.rows / 2 - (square / 2), square, square); //right mid
		Rect box6(frame.cols / 2 + square, frame.rows / 2 + (3 * square / 2), square, square); //right low

		//threshold the images
		begin = clock();
		threshImage[0] = scanBox(box1, frame, 0);
		threshImage[1] = scanBox(box2, frame, 1);
		threshImage[2] = scanBox(box3, frame, 2);
		threshImage[3] = scanBox(box4, frame, 3);
		threshImage[4] = scanBox(box5, frame, 4);
		threshImage[5] = scanBox(box6, frame, 5);
		tThreshold = double(clock() - begin) / CLOCKS_PER_SEC;

		//combine
		begin = clock();
		finImage = threshImage[0] | threshImage[1] | threshImage[2] | threshImage[3] | threshImage[4] | threshImage[5];
		tCombine = double(clock() - begin) / CLOCKS_PER_SEC;

		//get rid of excess noise
		begin = clock();
		medianBlur(finImage, finImage, 3);
		tMedBlur = double(clock() - begin) / CLOCKS_PER_SEC;

		//opening morphological transform
		begin = clock();
		erode(finImage, finImage, eroElement);
		dilate(finImage, finImage, dilElement);
		tMorph = double(clock() - begin) / CLOCKS_PER_SEC;

		//draw rectangles
		begin = clock();
		rectangle(showFrame, box1, Scalar(0, 0, 255), 2, 8, 0);
		rectangle(showFrame, box2, Scalar(0, 0, 255), 2, 8, 0);
		rectangle(showFrame, box3, Scalar(0, 0, 255), 2, 8, 0);
		rectangle(showFrame, box4, Scalar(0, 0, 255), 2, 8, 0);
		rectangle(showFrame, box5, Scalar(0, 0, 255), 2, 8, 0);
		rectangle(showFrame, box6, Scalar(0, 0, 255), 2, 8, 0);

		//display frame
		namedWindow("frame", CV_WINDOW_AUTOSIZE);
		imshow("frame", showFrame);

		//show results
		namedWindow("finImage", CV_WINDOW_AUTOSIZE);
		imshow("finImage", finImage);
		tDisplayResults = double(clock() - begin) / CLOCKS_PER_SEC;

		if (waitKey(30) == ESC_KEY){
			finFrame = finImage;
			break;
		}

	}

	cout << "TIMING: " << endl;
	cout << "Gauss: " << tGaussBlr << endl;
	cout << "tThreshold: " << tThreshold << endl;
	cout << "tCombine: " << tCombine << endl;
	cout << "tMedBlur: " << tMedBlur << endl;
	cout << "tMorph: " << tMorph << endl;
	cout << "tDisplayResults: " << tDisplayResults << endl;

	while (1){
		Mat frame, finImage, showFrame, threshImage[6];
		Scalar lowerCol, upperCol;

		//get frame from camera
		cap >> frame;
		flip(frame, frame, 1);

		//soften image
		GaussianBlur(frame, frame, Size(5, 5), 0);

		cvtColor(frame, frame, CV_BGR2HSV);

		//threshold the images
		for (int i = 0; i<numSamples; i++){
			Scalar lowerSkinCol, upperSkinCol;
			
			lowerSkinCol[0] = clamp(avgSkinCol[i][0] - 30);
			lowerSkinCol[1] = clamp(avgSkinCol[i][1] - 30);
			lowerSkinCol[2] = clamp(avgSkinCol[i][2] - 40);
			upperSkinCol[0] = clamp(avgSkinCol[i][0] + 30);
			upperSkinCol[1] = clamp(avgSkinCol[i][1] + 30);
			upperSkinCol[2] = clamp(avgSkinCol[i][2] + 40);

			inRange(frame,
				Scalar(lowerSkinCol[0], lowerSkinCol[1], lowerSkinCol[2]), //0,55,90
				Scalar(upperSkinCol[0], upperSkinCol[1], upperSkinCol[2]), //28,175,230
				threshImage[i]
				);

			//medianBlur(threshImage[i], threshImage[i], 5);
		}

		//combine
		finImage = threshImage[0] | threshImage[1] | threshImage[2] | threshImage[3] | threshImage[4] | threshImage[5];

		//get rid of excess noise
		medianBlur(finImage, finImage, (1 * 2) + 1);

		//opening morphological transform
		erode(finImage, finImage, eroElement);
		dilate(finImage, finImage, dilElement);

		//show results
		namedWindow("finImage", CV_WINDOW_AUTOSIZE);
		imshow("finImage", finImage);

		///save result
		finFrame = finImage;

		/** Find the biggest contour **/
		Mat canny_output;
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		double area, maxArea = 0;
		int maxContour, numFingers;

		/// Detect edges using canny
		//Canny(finFrame, canny_output, 80, 80 * 2, 3);
		/// Find contours
		findContours(finFrame, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		//find the maxmimum area contour
		for (int i = 0; i < contours.size(); i++){
			area = contourArea(contours[i]);
			if (area > maxArea){
				maxArea = area;
				maxContour = i;
			}
		}

		
		if (contours.size() >= 1){

			//find the convex hull
			vector<vector<Point>> hull(contours.size());
			vector<int> hullI;
			convexHull(Mat(contours[maxContour]), hull[maxContour], false); //hull to draw
			convexHull(Mat(contours[maxContour]), hullI, false); //hull to calculate defects

			//find centroid of hand
			Point centroid(0,0);
			for (int i = 0; i < hull[maxContour].size(); i++){
				centroid.x += hull[maxContour][i].x;
				centroid.y += hull[maxContour][i].y;
			}
			centroid.x /= hull[maxContour].size();
			centroid.y /= hull[maxContour].size();

			//find convex defects of hand
			vector<Vec4i> defects, fingerDefects;
			convexityDefects(contours[maxContour], hullI, defects); //find defects
			fingerDefects = findFingerDefects(defects, contours[maxContour]); //eliminate irrelevant defects

			///determine number of fingers
			vector<Point> fingerTips;
			int i = 0;
			vector<Vec4i>::iterator d = fingerDefects.begin();
			while (d != fingerDefects.end()) {
				Vec4i& v = (*d);
				int startidx = v[0]; 
				int endidx = v[1]; 
				int depthidx = v[2];
				Point ptStart(contours[maxContour][startidx]); 
				Point ptEnd(contours[maxContour][endidx]);
				Point ptDepth(contours[maxContour][depthidx]);

				if (i == 0){
					fingerTips.push_back(ptStart);
					i++;
				}
				fingerTips.push_back(ptEnd);
				d++;
				i++;
			}
			if (fingerTips.size() == 0){
				//checkForOneFinger(m);
				cout << "check for one Finger!" << endl;
			}

			/// Draw contours
			Mat drawing = Mat::zeros(finFrame.size(), CV_8UC3);
			drawContours(drawing, contours, maxContour, Scalar(0, 0, 255), 2, 8, hierarchy, 0, Point());
			drawContours(drawing, hull, maxContour, Scalar(0, 255, 0), 2, 8, hierarchy, 0, Point());
			///Draw Centroid
			circle(drawing, centroid, 5, Scalar(255, 0, 0), 10, 8, 0);

			///Optional Keyboard Input
			if (arrowKeyDemo){
				int leftBound = drawing.cols / 4;
				int rightBound = 3 * drawing.cols / 4;
				int upBound = drawing.rows / 4;

				if (centroid.x < leftBound && centroid.y < upBound) //up left
					upLeftDiag();
				else if (centroid.x > rightBound && centroid.y < upBound) //up right
					upRightDiag();
				else if (centroid.x < leftBound && centroid.y > upBound) //left straight
					leftPress();
				else if (centroid.x > rightBound && centroid.y > upBound) //right straight
					rightPress();
				else if (centroid.y < upBound)
					spacePress();
				else
					releaseKey();
			}
			
			///draw finger tips
			Point p;
			int k = 0;
			for (int i = 0; i<fingerTips.size(); i++){
				p = fingerTips[i];
				if (p.y < centroid.y)
					circle(drawing, p, 5, Scalar(100, 255, 100), 4);
			}

			/// Show in a window
			//string fingerCount = "fingers: " + to_string(numFingers);
			//putText(drawing, fingerCount, Point(20, 20), FONT_HERSHEY_PLAIN, 2, Scalar(255, 255, 0), 4, 8, 0);
			namedWindow("Contours", CV_WINDOW_AUTOSIZE);
			imshow("Contours", drawing);

		}
		if (waitKey(30) == ESC_KEY){
			break;
		}
	}
	
	return finFrame;
}

Mat scanBox(Rect box, Mat frame, int boxNum){
	Mat threshImage;

	avgSkinCol[boxNum] = detectSkinColor(frame, box);

	//GaussianBlur(frame, frame, Size(5,5), 0);
	Scalar lowerSkinCol, upperSkinCol;
	//check overflows - unnecessary?
	lowerSkinCol[0] = clamp(avgSkinCol[boxNum][0] - 30);
	lowerSkinCol[1] = clamp(avgSkinCol[boxNum][1] - 40);
	lowerSkinCol[2] = clamp(avgSkinCol[boxNum][2] - 30);
	upperSkinCol[0] = clamp(avgSkinCol[boxNum][0] + 30);
	upperSkinCol[1] = clamp(avgSkinCol[boxNum][1] + 40);
	upperSkinCol[2] = clamp(avgSkinCol[boxNum][2] + 30);

	//threshold skin
	inRange(frame,
		Scalar(lowerSkinCol[0], lowerSkinCol[1], lowerSkinCol[2]), //0,55,90
		Scalar(upperSkinCol[0], upperSkinCol[1], upperSkinCol[2]), //28,175,230
		threshImage
		);

	//medianBlur(threshImage, threshImage, 5);

	return threshImage;
}

Scalar detectSkinColor(Mat frame, Rect scanBox){

	//create Mat with ROI
	Mat skinFrame(frame, scanBox);
	//GaussianBlur(skinFrame, skinFrame, Size(15,15), 0);

	//find the average value
	Scalar avgSkinColor = mean(skinFrame); //perhaps get median?

	//cout << "avg Color" << avgSkinColor << endl;

	return avgSkinColor;
}

vector<Vec4i> findFingerDefects(vector<Vec4i> defects, vector<Point> contours){
	vector<Vec4i> fingerDefects;
	double minDepth = 10;
	double minAngle = 95;
	Vec4i currD; //used to track the current defect

	for (int i = 0; i < defects.size(); i++){
		//cout << "depth is: " << defects[i][3] << endl;
		currD = defects[i];
		if (currD[3] > minDepth && findAngle(contours[currD[0]], contours[currD[1]], contours[currD[2]]) > minAngle){
			fingerDefects.push_back(defects[i]);
		}
	}

	return fingerDefects;
}

float findAngle(Point start, Point end, Point depth){
	Point a(start.x - depth.x, start.y - depth.y); //vector a
	Point b(end.x - depth.x, end.y - depth.y); //vector b
	double magAB = sqrt(pow(a.x, 2) + pow(a.y, 2)) * sqrt(pow(b.x, 2) + pow(b.y, 2)); // |a||b|
	double dotAB = (a.x * b.x) + (a.y * b.y); // a dot b
	float angle = acos(dotAB / magAB); // arccos( a.b/|a||b|)
	angle = angle * 180 / PI; //conver to degrees for simplicity
	return angle;
}

int clamp(int n){
	int x = n > 255 ? 255 : n;
	x < 0 ? 0 : x;

	return x;
}

void leftPress(){
	// Press the "LEFT" key
	ip[0].ki.wVk = VK_LEFT; // virtual-key code for the left arrow key
	ip[0].ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip[0], sizeof(INPUT));
}

void rightPress(){
	// Press the "RIGHT" key
	ip[0].ki.wVk = VK_RIGHT; // virtual-key code for the right arrow key
	ip[0].ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip[0], sizeof(INPUT));
}

void spacePress(){
	ip[0].ki.wVk = VK_SPACE; // virtual-key code for the spacebar
	ip[0].ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip[0], sizeof(INPUT));
}

void upRightDiag(){
	// Press the spacebar
	ip[0].ki.wVk = VK_SPACE;
	ip[0].ki.dwFlags = 0;
	//press "RIGHT" key
	ip[1].ki.wVk = VK_RIGHT;
	ip[1].ki.dwFlags = 0;

	SendInput(2, ip, sizeof(INPUT));
}

void upLeftDiag(){
	// Press the spacebar
	ip[0].ki.wVk = VK_SPACE;
	ip[0].ki.dwFlags = 0;
	//press "LEFT" key
	ip[1].ki.wVk = VK_LEFT;
	ip[1].ki.dwFlags = 0;

	SendInput(2, ip, sizeof(INPUT));
}

void releaseKey(){
	// Release the key
	ip[0].ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
	ip[1].ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(2, ip, sizeof(INPUT));
}

void initKeyboard(){
	// Set up a generic keyboard event.
	ip[0].type = INPUT_KEYBOARD;
	ip[0].ki.wScan = 0; // hardware scan code for key
	ip[0].ki.time = 0;
	ip[0].ki.dwExtraInfo = 0;
	ip[1].type = INPUT_KEYBOARD;
	ip[1].ki.wScan = 0; // hardware scan code for key
	ip[1].ki.time = 0;
	ip[1].ki.dwExtraInfo = 0;
}
