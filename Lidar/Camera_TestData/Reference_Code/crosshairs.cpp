#include "opencv2/opencv.hpp"
using namespace cv;
int main(int argc, char** argv)
{
	VideoCapture cap;
	// open the default camera, use something different from 0 otherwise;
	// Check VideoCapture documentation.
	if (!cap.open(0))
		return 0;
	Mat frame;
	cap >> frame;
	assert(frame.channels() == 3);
	int xHair = frame.cols / 2;
	int yHair = frame.rows / 2;
	bool freeze = false;
	for (;;)
	{
		cap >> frame;
		for (int ixRow = 0; ixRow < frame.rows; ixRow++) {
			uchar *pRow = frame.ptr<uchar>(ixRow);
			if (ixRow == yHair) {
				for (int col = 0; col < frame.cols; col++) {
					//pRow[(col * frame.channels()) + 1] = 255;
					for (int channel = 0; channel < frame.channels(); channel++) {
						pRow[(col * frame.channels()) + channel] = 255 - pRow[(col * frame.channels()) + channel];
					}
				}
			} else {
				//pRow[(xHair * frame.channels()) + 1] = 255;
				for (int channel = 0; channel < frame.channels(); channel++) {
					pRow[(xHair * frame.channels()) + channel] = 255 - pRow[(xHair * frame.channels()) + channel];
				}
			}
		}
		if (frame.empty()) break; // end of video stream
		if (!freeze) {
			imshow("this is you, smile! :)", frame);
		}
		// if (waitKey(10) == 27) break; // stop capturing by pressing ESC 
		int key = cvWaitKey(1);		
		switch (key) {
			case 27:
				return 0;
			case 2424832:
				xHair = max(0, xHair - 30); break;
			case 2490368:
				yHair = max(0, yHair - 30); break;
			case 2555904:
				xHair = min(frame.cols - 1, xHair + 30); break;
			case 2621440:
				yHair = min(frame.rows - 1, yHair + 30); break;
			case 13:
				freeze = !freeze; break;
			case 255: 
			case -1:
				break;
			default:
				std::cout << key << std::endl;
		}
	}
	// the camera will be closed automatically upon exit
	// cap.close();
	return 0;
}
