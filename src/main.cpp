#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv.hpp>
#include <cv.h>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

int detectHand();
int HSVMethod ();
Scalar detectSkinColor(Mat frame, Rect scanBox);
Mat scanBox(Rect box, Mat frame);

int main(){

	detectHand();
	//HSVMethod();

	return 0;
}

int detectHand(){

	// open the default camera
    VideoCapture cap(0);

    // check if we succeeded
    if(!cap.isOpened())
        return -1;

    while(1){
    	Mat frame, showFrame, channel[3], threshImage[6], finImage;
		Scalar avgSkinCol[3], lowerCol, upperCol;

		cap >> frame;
		flip(frame, frame, 1);

		frame.copyTo(showFrame);

		//soften image
		GaussianBlur(frame, frame, Size(5,5), 0);

		//remove noise
		medianBlur(frame, frame, 5);
		cvtColor(frame, frame, CV_BGR2HSV);

		//determine dimensions
		int square = 25;
		//middle
		Rect box1(frame.cols/2 - square,
					 frame.rows/2 - square/2,
					 square,
					 square);
		//upper
		Rect box2(frame.cols/2 - square,
					 frame.rows/2 - (5*square/2),
					 square,
					 square);
		//lower
		Rect box3(frame.cols/2 - square,
					 frame.rows/2 + (3*square/2),
					 square,
					 square);

		Rect box4(frame.cols/2 + square,
							 frame.rows/2 - (square/2),
							 square,
							 square);

		Rect box5(frame.cols/2 + square,
							 frame.rows/2 - (5*square/2),
							 square,
							 square);

		Rect box6(frame.cols/2 + square,
							 frame.rows/2 + (3*square/2),
							 square,
							 square);

		threshImage[0] = scanBox(box1,frame);
		threshImage[1] = scanBox(box2,frame);
		threshImage[2] = scanBox(box3,frame);
		threshImage[3] = scanBox(box4,frame);
		threshImage[4] = scanBox(box5,frame);
		threshImage[5] = scanBox(box6,frame);

		finImage = threshImage[0] | threshImage[1] | threshImage[2] | threshImage[3] | threshImage[4] | threshImage[5];


		rectangle(showFrame,box1,Scalar(0,0,255),2,8,0);
		rectangle(showFrame,box2,Scalar(0,0,255),2,8,0);
		rectangle(showFrame,box3,Scalar(0,0,255),2,8,0);
		rectangle(showFrame,box4,Scalar(0,0,255),2,8,0);
		rectangle(showFrame,box5,Scalar(0,0,255),2,8,0);
		rectangle(showFrame,box6,Scalar(0,0,255),2,8,0);

		//display frame
		namedWindow("frame", CV_WINDOW_AUTOSIZE);
		imshow("frame", showFrame);

		//show results
		namedWindow("finImage", CV_WINDOW_AUTOSIZE);
		imshow("finImage", finImage);

        if(waitKey(30) >= 0)
            break;
    }

    return 0;
}

Mat scanBox(Rect box, Mat frame){
	Mat threshImage;
	Scalar avgSkinCol;

	avgSkinCol = detectSkinColor(frame, box);

	//GaussianBlur(frame, frame, Size(5,5), 0);
	//threshold skin
	inRange(frame,
			Scalar(avgSkinCol[0]-10,avgSkinCol[1]-10,180),
			Scalar(avgSkinCol[0]+10,avgSkinCol[1]+10,230),
			threshImage);

	medianBlur(threshImage, threshImage, 5);

	return threshImage;
}

Scalar detectSkinColor(Mat frame, Rect scanBox){

	//create Mat with ROI
	Mat skinFrame(frame,scanBox);
	GaussianBlur(skinFrame, skinFrame, Size(15,15), 0);

	//find the average value
	Scalar avgSkinColor = mean(skinFrame); //perhaps get median?

	cout << "avg Color" << avgSkinColor << endl;

	return avgSkinColor;

}
