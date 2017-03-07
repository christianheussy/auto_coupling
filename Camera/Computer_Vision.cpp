/**********************************
 **Using ZED with OpenCV
 **********************************/

#include <iostream>
#include <fstream>

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// ZED
#include <zed/Camera.hpp>

using namespace cv;
using namespace std;
void coordinateGrab(Mat contours, int x, int y, int& left, int& right, int& x_center, int& y_center);
void distanceGrab(float& l1, float& l2, int& left, int& right, int& y_center, sl::zed::Mat distancevalue);


int main(int argc, char** argv)
{	
	std::string Coupling = "/media/ubuntu/SDCARD/Coupling_2_short.svo";
	int depth_clamp = 20000;
	int last_key; //last key pressed by user
	ofstream mystream;
	mystream.open("/home/ubuntu/Documents/SeniorProject/TestData/Coupling2data3.txt");
	
	// Initialize ZED color stream in HD and depth in Performance mode
	sl::zed::Camera* zed = new sl::zed::Camera(Coupling);
	sl::zed::InitParams params;
	params.mode = sl::zed::MODE::QUALITY;
	params.device = -1;
	params.verbose = true;

	// Quit if an error occurred
	sl::zed::ERRCODE err = zed->init(params);
	if (err != sl::zed::SUCCESS) {
		std::cout << "Unable to init the ZED:" << errcode2str(err) << std::endl;
		delete zed;
		return 1;
	}
	
	// Set the Depth Clamp to an appropriate value (value is in milimeters)
	zed->setDepthClampValue(depth_clamp);

	// Initialize color image and depth
	int width = zed->getImageSize().width;
	int height = zed->getImageSize().height;
	cv::Mat left_image(height, width, CV_8UC4,1);
	cv::Mat left_no_crosshairs(height, width, CV_8UC4,1);
	cv::Mat right_image(height, width, CV_8UC4,1);
	cv::Mat distance(height, width, CV_8UC4,1);	

	// begin code for crosshairs
	//int xHair = left_image.cols / 2; //vertical crosshair
	//int yHair = left_image.rows / 2; //horizontal crosshair
	int xHair = 420;
	int yHair = 320;
	bool freeze = false;
	int count = 0;
	
	// keyboard and numpad values to move crosshairs (these may need to be adjusted for new keyboards, only works with NUMLOCK OFF)
	const int NUMPAD_LEFT = 65430;
	const int NUMPAD_UP = 65431;
	const int NUMPAD_RIGHT = 65432;
	const int NUMPAD_DOWN = 65433;
	const int NUMPAD_DEL = 65439;
	const int NUMPAD_ENTER= 65421;
	const int KEYBOARD_LEFT = 65361;
	const int KEYBOARD_UP = 65362;
	const int KEYBOARD_RIGHT = 65363;
	const int KEYBOARD_DOWN = 65364;
	const int KEYBOARD_ESC = 27;
	const int KEYBOARD_ENTER = 10;
	const int GRID_VALUE = 20; // number of pixels the crosshairs move with one button press
	
	


	for (;;)
	{		
		if (!zed->grab(sl::zed::SENSING_MODE::FILL))
		{
			// get left image and and convert to cv mat (left_image)
			sl::zed::Mat left = zed->retrieveImage(sl::zed::SIDE::LEFT);
			//sl::zed::Mat left = zed->getView(sl::zed::VIEW_MODE::STEREO_OVERLAY);
			sl::zed::slMat2cvMat(left).copyTo(left_image);
			// get left image to display without crosshairs (left_no_crosshairs)
			sl::zed::Mat left_no_crosshairs1 = zed->retrieveImage(sl::zed::SIDE::LEFT);
			sl::zed::slMat2cvMat(left_no_crosshairs1).copyTo(left_no_crosshairs);
			// get right image and and convert to cv mat (right_image)
			sl::zed::Mat right = zed->retrieveImage(sl::zed::SIDE::RIGHT);
			sl::zed::slMat2cvMat(right).copyTo(right_image);
			//sl::zed::Mat depthmap = zed->normalizeMeasure(sl::zed::MEASURE::DEPTH);
		} else {
			return 1;
		}
		
		// create the crosshairs
		for (int ixRow = 0; ixRow < left_image.rows; ixRow++) {
			uchar *pRow = left_image.ptr<uchar>(ixRow);
			if (ixRow == yHair) {
				for (int col = 0; col < left_image.cols; col++) {
					for (int channel = 0; channel < left_image.channels(); channel++) {
						pRow[(col * left_image.channels()) + channel] = 255 - pRow[(col * left_image.channels()) + channel];
					}
				}
			} else {
				for (int channel = 0; channel < left_image.channels(); channel++) {
					pRow[(xHair * left_image.channels()) + channel] = 255 - pRow[(xHair * left_image.channels()) + channel];
				}
			}	
		}
	
		if (left_image.empty()) break; // end of video stream
		if (!freeze) {
			imshow("this is you, smile! :)", left_image);

		}
		
		// move the crosshairs with the arrow keys
		int key = cvWaitKey(1);
		if(key == KEYBOARD_ENTER || key == NUMPAD_ENTER)		
			last_key = key;
		switch (key) {
			// move left
			case KEYBOARD_LEFT:
			case NUMPAD_LEFT:
				xHair = max(0, xHair - GRID_VALUE); break;
			// move up
			case KEYBOARD_UP:
			case NUMPAD_UP:
				yHair = max(0, yHair - GRID_VALUE); break;
			// move right
			case KEYBOARD_RIGHT:
			case NUMPAD_RIGHT:
				xHair = min(left_image.cols - 1, xHair + GRID_VALUE); break;
			// move down
			case KEYBOARD_DOWN:
			case NUMPAD_DOWN:
				yHair = min(left_image.rows - 1, yHair + GRID_VALUE); break;
			// don't display these output values in the terminal
			case -1:
			case KEYBOARD_ESC:
			case NUMPAD_DEL:
				break;
			// display the numeric value of any key pressed if it wasn't part of the cases above
			//default:
				//std::cout << key << std::endl;
		}
		
		//sl::zed::Mat depthZed;
		//depthZed = zed->normalizeMeasure(sl::zed::MEASURE::DEPTH);
		//Mat depthCv(height, width, CV_8UC4,1)
		
		int thresh = 30;
		int blur = 1;
		int pixel_shift = 3;
				
		//count so this section doesn't happen every time
		count++;
			
			if(count == 20){
				count = 0;
			//edge detection left image
			Mat left_edges(height, width, CV_8UC4,1);
			cvtColor(left_no_crosshairs, left_edges, COLOR_BGR2GRAY);
			GaussianBlur(left_edges, left_edges, Size(7,7), blur, blur);
			Canny(left_edges, left_edges, thresh, thresh*3, 3);
			
			//contour detection left image
			Mat l_contours = Mat::zeros(left_edges.rows, left_edges.cols, CV_8UC4);
		    vector<vector<Point> > left_contours;
		    vector<Vec4i> left_hierarchy;
		    findContours(left_edges, left_contours, left_hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
		    // draw each connected component with its own random color
		    int i = 0;
		    Scalar color(rand()&255, rand()&255, rand()&255);
		    for( ; i >= 0; i = left_hierarchy[i][0] ){
		        drawContours(l_contours, left_contours, i, color, CV_FILLED, 8, left_hierarchy);
		    }
		    //namedWindow( "Left Contours", 1);
		    //imshow( "Left Contours", l_contours);
		    
		/*    
		    //edge detection right image
			Mat right_edges(height, width, CV_8UC4,1);
			cvtColor(right_image, right_edges, COLOR_BGR2GRAY);
			GaussianBlur(right_edges, right_edges, Size(7,7), blur, blur);
			Canny(right_edges, right_edges, thresh, thresh*3, 3);
	
			
			//contour detection right image
			Mat r_contours = Mat::zeros(right_edges.rows, right_edges.cols, CV_8UC4);
		    vector<vector<Point> > right_contours;
		    vector<Vec4i> right_hierarchy;
		
		    findContours(right_edges, right_contours, right_hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
		
		    // draw each connected component with its own random color
		    int j = 0;
		    for( ; j >= 0; j = right_hierarchy[j][0] ){
		        drawContours(r_contours, right_contours, j, color, CV_FILLED, 8, right_hierarchy);
		    }
		*/
		
			// do something when enter is pressed (on keyboard or numpad)
			if (last_key == KEYBOARD_ENTER || last_key == NUMPAD_ENTER) {
				
				int leftcamera_leftedge, leftcamera_rightedge, x_center, y_center;
				//float l1 = 20000, l2 = 20000;
				float l1,l2;
				int left_coord, right_coord;
				//cout << "xhair= " << xHair << endl << "yhair= " << yHair << endl;
			
				
				// get the distance at the crosshair location
				sl::zed::Mat distancevalue;
				Mat distancevalue_cv(height, width, CV_8UC4,1);
				zed->grab(sl::zed::SENSING_MODE::FILL);
				distancevalue = zed->retrieveMeasure(sl::zed::MEASURE::DEPTH);
				
				/*
				float* ptr_image_num = (float*) ((int8_t*) distancevalue.data + yHair*distancevalue.step);
				float depth_in_mm = ptr_image_num[xHair];
				//std::cout << "depth = " << depth_in_mm << std::endl;
				*/
							
				coordinateGrab(l_contours, xHair, yHair, leftcamera_leftedge, leftcamera_rightedge, x_center, y_center);
				//coordinateGrab(r_contours, xHair, yHair, rightcamera_leftedge, rightcamera_rightedge, x_center, y_center);
				
				left_coord = leftcamera_leftedge;
				left_coord += pixel_shift;
				right_coord = leftcamera_rightedge;
				right_coord -= pixel_shift;
				
				//float l1_prev = l1;
				//float l2_prev = l2;
				
				distanceGrab(l1, l2, left_coord, right_coord, y_center, distancevalue);
			
				/*
				while(l1 > l1_prev){
					left_coord++;
					distanceGrab(l1, l2, left_coord, right_coord, y_center, distancevalue);
					//std::cout << "INF!!!! l1= " << l1 <<  std::endl;
				}
				while(l2 > l2_prev){
					right_coord--;
					distanceGrab(l1, l2, left_coord, right_coord, y_center, distancevalue);
					//std::cout << "INF!!! l2= " << l2 << std::endl;
				}
				*/
				
				
				if(min(l1, l2) < 10000)
					pixel_shift = 4;
				else if(min(l1,l2) < 7000)
					pixel_shift = 5;
					
				float l1_meters = l1 / 1000;
				float l2_meters = l2 / 1000;	
				mystream << "l1= " << l1_meters << " meters" << std::endl 
						<< "l2= " << l2_meters << " meters" << std::endl << std::endl;
			
				xHair = x_center;
				yHair = y_center;
				if(max(l1, l2) < 20000)
					depth_clamp = max(l1, l2) + 1000;	
				zed->setDepthClampValue(depth_clamp);		
			} 
		}
	}
	mystream.close();
	delete zed;
	return 0;
}

//figure out parameters for coordinate and distance grab

void coordinateGrab(Mat contours, int x, int y, int& left, int& right, int& x_center, int& y_center)
{
	int top = y;
	int bottom = y;
	left = x;
	right = x;
		
	int* top_mat = (int*) ((int8_t*) contours.data + top*contours.step);
	int top_num = top_mat[x];
	while(top_num == 0 || top_num == -1){
		if(top <= 0)
			break;
		top--;
		top_mat = (int*) ((int8_t*) contours.data + top*contours.step);
		top_num = top_mat[x];
	}
	int* bottom_mat = (int*) ((int8_t*) contours.data + bottom*contours.step);
	int bottom_num = bottom_mat[x];
	while(bottom_num == 0 || bottom_num == -1){
		if(bottom >= 720)
			break;
		bottom++;
		bottom_mat = (int*) ((int8_t*) contours.data + bottom*contours.step);
		bottom_num = bottom_mat[x];
	}
	int* left_mat = (int*) ((int8_t*) contours.data + y*contours.step);
	int left_num = left_mat[left];
	while(left_num == 0 || left_num == -1){
		if(left < 0)
			break;
		left--;
		left_num = left_mat[left];
	}
	int* right_mat = (int*) ((int8_t*) contours.data + y*contours.step);
	int right_num = right_mat[right];                                
	while(right_num == 0 || right_num == -1){
		if(right >= 1080)
			break;
		right++;
		right_num = right_mat[right];
	}
		
	y_center = (top+bottom)/2;
	x_center = (left+right)/2;
	
	
	/*
	std::cout << "xHair: " << x << std::endl;
	std::cout << "yHair: " << y << std::endl;
	std::cout << "top: " << top << std::endl;
	std::cout << "bottom: " << bottom << std::endl;
	std::cout << "left: " << left << std::endl;
	std::cout << "right: " << right << std::endl;
	std::cout << "x_center: " << x_center << std::endl;
	std::cout << "y_center: " << y_center << std::endl;
	*/
			
}

void distanceGrab(float& l1, float& l2, int& left, int& right, int& y_center, sl::zed::Mat distancevalue)
{
	float* left_edge_num = (float*) ((int8_t*) distancevalue.data + y_center*distancevalue.step);
	l1 = left_edge_num[left];
		
	float* right_edge_num = (float*) ((int8_t*) distancevalue.data + y_center*distancevalue.step);
	l2 = right_edge_num[right];
	
}


		
