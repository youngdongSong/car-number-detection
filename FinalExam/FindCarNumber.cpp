#include <opencv2/opencv.hpp>
#include<opencv\cv.h>
#include<opencv\cxcore.h>
#include<opencv\/cvaux.h>
#include<opencv\highgui.h>
#include<stdio.h>

using namespace cv;
using namespace std;
#define _CRT_SECURE_NO_WARNINGS

int gCameraFlag = 0;

void mouseHandlerFunc(int, int, int, int, void*);

int main(int argc, char *argv[]) {

	
	Mat image, image2, image3, drawing;  
	Rect rect, temp_rect;  
	vector<vector<Point> > vp; 
	vector<Vec4i> ch;

	double r, delta_x, delta_y, gradient;  
	int select, plate_width, count, friend_count = 0, refinery_count = 0;

	
	const char *winNameIn = "original image";
	int c, param = 3;
	IplImage *pImgIpl = 0;
	CvCapture *pCamera;
	char savename[50];
	char saveroot[150] = "C:/Users/sos94/Desktop/";

	if (!(pCamera = cvCaptureFromCAM(0))) {
		printf("can't connect Cam\n");
		return -1;
	}

	cvNamedWindow(winNameIn, CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback(winNameIn, mouseHandlerFunc, &param);

	while (!0) {
		if (gCameraFlag) {
			pImgIpl = cvQueryFrame(pCamera);
			cvShowImage(winNameIn, pImgIpl);
		}
		c = cvWaitKey(10);

		if ((char)c == 27)
			break;
		if ((char)c == 99) {
			printf("Save Image name : ");
			scanf("%s", savename);
			strcat(savename, ".jpg");
			strcat(saveroot, savename);
			cvSaveImage(saveroot, pImgIpl);
			printf("Capture succuess\n");
		
			image = imread(saveroot);
			imshow("Original", image);
			waitKey(0);

			image.copyTo(image2);  
			image.copyTo(image3);  

			cvtColor(image2, image2, CV_BGR2GRAY);  
			Canny(image2, image2, 100, 300, 3);  
			
			findContours(image2, vp, ch, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point());
			vector<vector<Point> > vp2(vp.size());
			vector<Rect> vRect(vp.size());
			vector<Rect> vRect2(vp.size());

			
			for (int i = 0; i < vp.size(); i++) {
				approxPolyDP(Mat(vp[i]), vp2[i], 1, true);
				vRect[i] = boundingRect(Mat(vp2[i]));
			}

			drawing = Mat::zeros(image2.size(), CV_8UC3);

			for (int i = 0; i < vp.size(); i++) {

				r = (double)vRect[i].height / vRect[i].width;

				
				if ((r <= 2.5) && (r >= 0.5) && (vRect[i].area() <= 700) && (vRect[i].area() >= 100)) {

					drawContours(drawing, vp, i, Scalar(0, 255, 255), 1, 8, ch, 0, Point());
					rectangle(drawing, vRect[i].tl(), vRect[i].br(), Scalar(255, 0, 0), 1, 8, 0);

					
					vRect2[refinery_count] = vRect[i];
					refinery_count += 1;
				}
			}

			vRect2.resize(refinery_count);

			waitKey(0);


			//STL sort함수를 이용한 버블 정렬
			struct compare_obj {
				bool operator ()(const cv::Rect& rect1, const cv::Rect& rect2) {
					return rect1.tl().x < rect2.tl().x;
				}
			};
			


			for (int i = 0; i < vRect2.size(); i++) {

				rectangle(image3, vRect2[i].tl(), vRect2[i].br(), Scalar(0, 255, 0), 1, 8, 0);
				count = 0;

				for (int j = i + 1; j < vRect2.size(); j++) {
					delta_x = abs(vRect2[j].tl().x - vRect2[i].tl().x);
					
					if (delta_x > 150)  
						break;

					delta_y = abs(vRect2[j].tl().y - vRect2[i].tl().y);


					
					if (delta_x == 0) {
						delta_x = 1;
					}

					if (delta_y == 0) {
						delta_y = 1;
					}


					gradient = delta_y / delta_x;  
					

					if (gradient < 0.25) {  
						count += 1;
					}
				}

				
				if (count > friend_count) {
					select = i;  
					friend_count = count;  
					rectangle(image3, vRect2[select].tl(), vRect2[select].br(), Scalar(255, 0, 0), 1, 8, 0);
					plate_width = delta_x;  
				}                          
			}

			
			rectangle(image3, vRect2[select].tl(), vRect2[select].br(), Scalar(0, 0, 255), 2, 8, 0);
			line(image3, vRect2[select].tl(), Point(vRect2[select].tl().x + plate_width, vRect2[select].tl().y), Scalar(0, 0, 255), 1, 8, 0);
			
			imshow("Result", image(Rect(vRect2[select].tl().x - 60, vRect2[select].tl().y - 20, plate_width + 40, plate_width*0.3)));
			waitKey(0);

			imwrite("C:/Users/sos94/Desktop/car_num.jpg",
				image(Rect(vRect2[select].tl().x - 60, vRect2[select].tl().y - 20, plate_width + 40, plate_width*0.3)));


			
			
		}

	}

	cvReleaseCapture(&pCamera);
	cvDestroyWindow(winNameIn);

	return 0;
}

void mouseHandlerFunc(int event, int x, int y, int flags, void *paran) {
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		printf("L-button clicked, start capture\n");
		gCameraFlag = 1;
		break;
	case CV_EVENT_RBUTTONDOWN:
		printf("R-button clicked, end capture\n");
		gCameraFlag = 0;
		break;
	}
}