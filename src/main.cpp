#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv.hpp>
#include <cv.h>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

int erosion_elem = 0;
int erosion_size = 2;
int dilation_elem = 0;
int dilation_size =5;
int const max_elem = 2;
int const max_kernel_size = 21;


Mat detectHand(VideoCapture cap);
Scalar detectSkinColor(Mat frame, Rect scanBox);
Mat scanBox(Rect box, Mat frame);

int main(){
	Mat frame;

	// open the default camera
	VideoCapture cap(0);

	// check if we succeeded
	if(!cap.isOpened())
		return -1;

	frame = detectHand(cap);
	//HSVMethod();

	 Mat canny_output, src_gray;
	  vector<vector<Point> > contours;
	  vector<Vec4i> hierarchy;

	  /// Find contours
	  findContours( frame, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

	  /// Draw contours
	  Mat drawing = Mat::zeros( frame.size(), CV_8UC3 );
	  for( int i = 0; i< contours.size(); i++ )
	     {
	       Scalar color = Scalar(0,0,255);
	       drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
	     }

	  /// Show in a window
	  namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
	  imshow( "Contours", drawing );

	waitKey(0);

	return 0;
}

Mat detectHand(VideoCapture cap){
	Mat finFrame;


    while(1){
    	Mat frame, finImage, showFrame, channel[3], threshImage[6];
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

		//upper
		Rect box1(frame.cols/2 - square/2,
					 frame.rows/2 - (square/2),
					 square,
					 square);

		//middle
		Rect box2(frame.cols/2 - square/2,
					 frame.rows/2 - square/2,
					 square,
					 square);

		//left middle
		Rect box3(frame.cols/2 - 2*square,
					 frame.rows/2 - (square/2),
					 square,
					 square);

		//left lower
		Rect box4(frame.cols/2 - 2*square,
				 frame.rows/2 + (3*square/2),
				 square,
				 square);

		//right middle
		Rect box5(frame.cols/2 + square,
							 frame.rows/2 - (square/2),
							 square,
							 square);

		Rect box6(frame.cols/2 + square,
							 frame.rows/2 + (3*square/2),
							 square,
							 square);

		Rect box7(frame.cols/2 - square,
					 frame.rows/2 + (3*square/2),
					 square,
					 square);

		threshImage[0] = scanBox(box1,frame);
		//threshImage[1] = scanBox(box2,frame);
		//threshImage[2] = scanBox(box3,frame);
		//threshImage[3] = scanBox(box4,frame);
		//threshImage[4] = scanBox(box5,frame);
		//threshImage[5] = scanBox(box6,frame);

		finImage = threshImage[0] | threshImage[1] | threshImage[2] | threshImage[3] | threshImage[4] | threshImage[5];


		Mat element1 = getStructuringElement( MORPH_RECT,
					   Size( 2*erosion_size + 1, 2*erosion_size+1 ),
					   Point( erosion_size, erosion_size ) );

		Mat element2 = getStructuringElement( MORPH_RECT,
							   Size( 2*dilation_size + 1, 2*dilation_size+1 ),
							   Point( dilation_size, dilation_size ) );

		//get rid of excess noise
		medianBlur(finImage,finImage,5);

		//opening morphological transform
		erode( finImage, finImage, element1);
		dilate( finImage, finImage, element2);

		rectangle(showFrame,box1,Scalar(0,0,255),2,8,0);
		//rectangle(showFrame,box2,Scalar(0,0,255),2,8,0);
		//rectangle(showFrame,box3,Scalar(0,0,255),2,8,0);
		//rectangle(showFrame,box4,Scalar(0,0,255),2,8,0);
		//rectangle(showFrame,box5,Scalar(0,0,255),2,8,0);
		//rectangle(showFrame,box6,Scalar(0,0,255),2,8,0);

		//display frame
		namedWindow("frame", CV_WINDOW_AUTOSIZE);
		imshow("frame", showFrame);

		//show results
		namedWindow("finImage", CV_WINDOW_AUTOSIZE);
		imshow("finImage", finImage);

        if(waitKey(30) >= 0){
        	finFrame = finImage;
        	break;
        }

    }

    return finFrame;
}

Mat scanBox(Rect box, Mat frame){
	Mat threshImage;
	Scalar avgSkinCol;

	avgSkinCol = detectSkinColor(frame, box);

	//GaussianBlur(frame, frame, Size(5,5), 0);
	//threshold skin
	inRange(frame,
			Scalar(avgSkinCol[0]-20,avgSkinCol[1]-30,140),
			Scalar(avgSkinCol[0]+20,avgSkinCol[1]+30,220),
			threshImage);

	medianBlur(threshImage, threshImage, 5);

	return threshImage;
}

Scalar detectSkinColor(Mat frame, Rect scanBox){

	//create Mat with ROI
	Mat skinFrame(frame,scanBox);
	//GaussianBlur(skinFrame, skinFrame, Size(15,15), 0);

	//find the average value
	Scalar avgSkinColor = mean(skinFrame); //perhaps get median?

	cout << "avg Color" << avgSkinColor << endl;

	return avgSkinColor;
}
