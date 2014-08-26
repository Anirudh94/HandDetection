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
Scalar avgSkinCol[6];
int numBox = 6;

//create elements for morphological tranforms
Mat eroElement = getStructuringElement( MORPH_RECT,
			   Size( 2*erosion_size + 1, 2*erosion_size+1 ),
			   Point( erosion_size, erosion_size ) );

Mat dilElement = getStructuringElement( MORPH_RECT,
					   Size( 2*dilation_size + 1, 2*dilation_size+1 ),
					   Point( dilation_size, dilation_size ) );

Mat filterHand(VideoCapture cap);
Scalar detectSkinColor(Mat frame, Rect scanBox);
Mat scanBox(Rect box, Mat frame, int boxNum);
void detectHand(Mat frame);

int main(){
	Mat frame;

	// open the default camera
	VideoCapture cap(0);

	// check if we succeeded
	if(!cap.isOpened())
		return -1;

	frame = filterHand(cap);

	detectHand(frame);
	//HSVMethod();

	waitKey(0);

	return 0;
}

void detectHand(Mat frame){

}

Mat filterHand(VideoCapture cap){
	Mat finFrame;

    while(1){
    	Mat frame, finImage, showFrame, channel[3], threshImage[6];

		//get frame from camera
		cap >> frame;
		flip(frame, frame, 1);

		//make a copy
		frame.copyTo(showFrame);

		//soften image
		GaussianBlur(frame, frame, Size(5,5), 0);

		cvtColor(frame, frame, CV_BGR2HSV);

		//Create ROIs
		int square = 25;
		Rect box1(frame.cols/2 - square/2, frame.rows/2 - (5*square/2),square, square); //up
 		Rect box2(frame.cols/2 - square/2, frame.rows/2 - square/2, square, square); //mid
		Rect box3(frame.cols/2 - 2*square, frame.rows/2 - (square/2), square, square); //left mid
		Rect box4(frame.cols/2 - 2*square, frame.rows/2 + (3*square/2), square, square); //left low
		Rect box5(frame.cols/2 + square, frame.rows/2 - (square/2), square, square); //right mid
		Rect box6(frame.cols/2 + square, frame.rows/2 + (3*square/2), square, square); //right low

		//threshold the images
		threshImage[0] = scanBox(box1,frame, 0);
		threshImage[1] = scanBox(box2,frame, 1);
		threshImage[2] = scanBox(box3,frame, 2);
		threshImage[3] = scanBox(box4,frame, 3);
		threshImage[4] = scanBox(box5,frame, 4);
		threshImage[5] = scanBox(box6,frame, 5);

		//combine
		finImage = threshImage[0] | threshImage[1] | threshImage[2] | threshImage[3] | threshImage[4] | threshImage[5];

		//get rid of excess noise
		medianBlur(finImage,finImage,5);

		//opening morphological transform
		erode( finImage, finImage, eroElement);
		dilate( finImage, finImage, dilElement);

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

        if(waitKey(30) >= 0){
        	finFrame = finImage;

        	break;
        }

    }

    while(1){
        	Mat frame, finImage, showFrame, threshImage[6];
    		Scalar lowerCol, upperCol;

    		//get frame from camera
    		cap >> frame;
    		flip(frame, frame, 1);

    		//soften image
    		GaussianBlur(frame, frame, Size(5,5), 0);

    		cvtColor(frame, frame, CV_BGR2HSV);

    		//threshold the images
    		for(int i = 0; i<numBox; i++){

        		inRange(frame,
        				Scalar(avgSkinCol[i][0]-20,avgSkinCol[i][1]-30,avgSkinCol[i][2]-20), //0,55,90
        				Scalar(avgSkinCol[i][0]+20,avgSkinCol[i][1]+30,avgSkinCol[i][2]+20), //28,175,230
        				threshImage[i]
        		);

        		medianBlur(threshImage[i], threshImage[i], 5);
    		}

    		//combine
    		finImage = threshImage[0] | threshImage[1] | threshImage[2] | threshImage[3] | threshImage[4] | threshImage[5];

    		//get rid of excess noise
    		medianBlur(finImage,finImage,11);

    		//opening morphological transform
    		erode( finImage, finImage, eroElement);
    		dilate( finImage, finImage, dilElement);

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

Mat scanBox(Rect box, Mat frame, int boxNum){
	Mat threshImage;

	avgSkinCol[boxNum] = detectSkinColor(frame, box);

	//GaussianBlur(frame, frame, Size(5,5), 0);
	//threshold skin
	inRange(frame,
			Scalar(avgSkinCol[boxNum][0]-20,avgSkinCol[boxNum][1]-30,avgSkinCol[boxNum][2]-20),
			Scalar(avgSkinCol[boxNum][0]+20,avgSkinCol[boxNum][1]+30,avgSkinCol[boxNum][2]+20),
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
