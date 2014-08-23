#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv.hpp>
#include <cv.h>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

int detectHand();
Scalar detectSkinColor(Mat frame, Rect scanBox);
Mat scanBox(Rect box, Mat frame);

int main(){

	detectHand();

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
		split(frame,channel);
		flip(frame, frame, 1);

		frame.copyTo(showFrame);

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

		//show results

		/** REctangles showing up! Whyyy?? **/
		//draw box
		rectangle(showFrame,box1,Scalar(0,0,255),2,8,0);
		rectangle(showFrame,box2,Scalar(0,0,255),2,8,0);
		rectangle(showFrame,box3,Scalar(0,0,255),2,8,0);
		rectangle(showFrame,box4,Scalar(0,0,255),2,8,0);
		rectangle(showFrame,box5,Scalar(0,0,255),2,8,0);
		rectangle(showFrame,box6,Scalar(0,0,255),2,8,0);

		//display frame
		namedWindow("frame", CV_WINDOW_AUTOSIZE);
		imshow("frame", showFrame);

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

	GaussianBlur(frame, frame, Size(5,5), 0);
	//threshold skin
	inRange(frame,
			Scalar(avgSkinCol[0]-10,avgSkinCol[1]-10,avgSkinCol[2]-10),
			Scalar(avgSkinCol[0]+10,avgSkinCol[1]+10,avgSkinCol[2]+10),
			threshImage);

	medianBlur(threshImage, threshImage, 5);

	return threshImage;
}

Scalar detectSkinColor(Mat frame, Rect scanBox){

	uchar redl,bluel,greenl;
	uchar redu,blueu,greenu;
	Vector<uchar> rPixels, bPixels, gPixels;

	//create Mat with ROI
	Mat skinFrame(frame,scanBox);
	GaussianBlur(skinFrame, skinFrame, Size(15,15), 0);

	namedWindow("snap", CV_WINDOW_AUTOSIZE);
	imshow("snap", skinFrame);

	//find the average value
	Scalar avgSkinColor = mean(skinFrame); //perhaps get median?

	//create a vector of pixels
	for(int i = 0; i < scanBox.height; i++){
	    for(int j = 0; j < scanBox.width; j++){
	        Vec3b bgrPixel = frame.at<Vec3b>(i, j);
	        bPixels.push_back(bgrPixel[0]);
	        gPixels.push_back(bgrPixel[1]);
	        rPixels.push_back(bgrPixel[2]);
	    }
	}

	std::sort(bPixels.begin(),bPixels.end());
	std::sort(gPixels.begin(),gPixels.end());
	std::sort(rPixels.begin(),rPixels.end());

	//find 5th percentile
	redl = rPixels[(rPixels.size()*0.1)];
	bluel = rPixels[(bPixels.size()*0.1)];
	greenl = rPixels[(gPixels.size()*0.1)];

	//find 95th percentile
	redu = rPixels[(rPixels.size()*0.9)];
	blueu = rPixels[(bPixels.size()*0.9)];
	greenu = rPixels[(gPixels.size()*0.9)];

	//lowerBound = Scalar(bluel,greenl,redl);
	//upperBound = Scalar(blueu,greenu,redu);

	//show output

	//cout << "lower Color:" << lowerBound << endl;
	cout << "avg Color" << avgSkinColor << endl;
	//cout << "upper Color:" << upperBound << endl;

	return avgSkinColor;

}
