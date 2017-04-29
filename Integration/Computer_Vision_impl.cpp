#include <cmath>
#include "Computer_Vision.h"
#include "config.h"

using namespace sl;


// keyboard and numpad values to move crosshairs
// these may need to be adjusted for new keyboards, only works with NUMLOCK OFF
const int NUMPAD_LEFT = 65430;
const int NUMPAD_UP = 65431;
const int NUMPAD_RIGHT = 65432;
const int NUMPAD_DOWN = 65433;
const int NUMPAD_DEL = 65439;
const int NUMPAD_ENTER = 65421;
const int KEYBOARD_LEFT = 65361;
const int KEYBOARD_UP = 65362;
const int KEYBOARD_RIGHT = 65363;
const int KEYBOARD_DOWN = 65364;
const int KEYBOARD_ESC = 27;
const int KEYBOARD_ENTER = 10;
const int GRID_VALUE = 20;

bool adjustCrosshairsByInput(int &x, int &y, int nRows, int nCols) {
	using std::max;
	using std::min;

	// initialize key with cvWaitKey (an openCV function reads numeric values corresponding to key presses)
	int key = cvWaitKey(1);
	
	switch (key) {
		// the user is finished moving the crosshairs
		case KEYBOARD_ENTER:
		case NUMPAD_ENTER:
			return true;
		// move left
		case KEYBOARD_LEFT:
		case NUMPAD_LEFT:
			x = max(0, x - GRID_VALUE); break;
		// move up
		case KEYBOARD_UP:
		case NUMPAD_UP:
			y = max(0, y - GRID_VALUE); break;
		// move right
		case KEYBOARD_RIGHT:
		case NUMPAD_RIGHT:
			x = min(nCols - 1, x + GRID_VALUE); break;
		// move down
		case KEYBOARD_DOWN:
		case NUMPAD_DOWN:
			y = min(nRows - 1, y + GRID_VALUE); break;
		// don't display these output values in the terminal
		case KEYBOARD_ESC:
		case NUMPAD_DEL:
			exit(0);
		case -1:
			break;
		//default:
			//std::cout << key << std::endl;
	}
	return false;
}

void drawCrosshairsInMat(cv::Mat &mat, int x, int y) {
	// create the crosshairs
	for (int ixRow = 0; ixRow < mat.rows; ixRow++) {
		uchar *pRow = mat.ptr<uchar>(ixRow);
		if (ixRow == y) {
			for (int col = 0; col < mat.cols; col++) {
				for (int channel = 0; channel < mat.channels(); channel++) {
					pRow[(col * mat.channels()) + channel] = 255 - pRow[(col * mat.channels()) + channel];
				}
			}
		} else {
			for (int channel = 0; channel < mat.channels(); channel++) {
				pRow[(x * mat.channels()) + channel] = 255 - pRow[(x * mat.channels()) + channel];
			}
		}	
	}
}

void detectEdgesAndContours(cv::Mat original_image, cv::Mat &image_contours, int image_height, int image_width, int threshold_value, int blur_value) {
	//edge detection
	cv::Mat edges(image_height, image_width, CV_8UC4,1);
	cvtColor(original_image, edges, cv::COLOR_BGR2GRAY);
	GaussianBlur(edges, edges, cv::Size(7,7), blur_value, blur_value);
	Canny(edges, edges, threshold_value, threshold_value*3, 3);

	//contour detection
	image_contours = cv::Mat::zeros(edges.rows, edges.cols, CV_8UC4);
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	findContours(edges, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	int i = 0;
	cv::Scalar color(rand()&255, rand()&255, rand()&255);
	for( ; i >= 0; i = hierarchy[i][0] ){
		drawContours(image_contours, contours, i, color, CV_FILLED, 8, hierarchy);
	}
	
	
}	

void coordinateGrab(cv::Mat contours, int x, int y, int& left, int& right, int& x_center, int& y_center)
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
	
}

void distanceGrab(float& l1, float& l2, int& left, int& right, int& y_center, sl::Mat distancevalue)
{
	sl::float4 l1_point3D;
	distancevalue.getValue(left, y_center, &l1_point3D, sl::MEM_CPU);
	float x1 = l1_point3D.x;
	float y1 = l1_point3D.y;
	float z1 = l1_point3D.z;
	l1 = sqrt(x1*x1 + z1*z1);
	
	sl::float4 l2_point3D;
	distancevalue.getValue(right, y_center, &l2_point3D, sl::MEM_CPU);
	float x2 = l2_point3D.x;
	float y2 = l2_point3D.y;
	float z2 = l2_point3D.z;
	l2 = sqrt(x2*x2 + z2*z2);
	
	//std::cout << "x1= " << l1_point3D.x << std::endl;
	//std::cout << "y1= " << l1_point3D.y << std::endl;
	//std::cout << "z1= " << l1_point3D.z << std::endl;
	//std::cout << "x2= " << l2_point3D.x << std::endl;
	//std::cout << "y2= " << l2_point3D.y << std::endl;
	//std::cout << "z2= " << l2_point3D.z << std::endl;
	
}


void pathInputCalculations_Camera(float left_dist, float right_dist, float& center_dist, float& theta_1, float& theta_2, int leftEdgeCoord, int rightEdgeCoord)
{
	const float w = WID;
	float theta_n;
	float theta_t;
	float x;
	const float y = 640.0;
	float theta_b;
	const float theta_c = 55.0*(M_PI/180.0);
	
	

		theta_n = acosf((w*w + right_dist*right_dist - left_dist*left_dist)/(2 * right_dist * w));
		//std::cout << "tn= " << theta_n << std::endl;
		center_dist = sqrt(w*w/4 + right_dist*right_dist - right_dist*w*cosf(theta_n));
		
		theta_t = acosf((w*w/4 + center_dist*center_dist - right_dist*right_dist)/(center_dist * w));
		std::cout << std::endl << std::endl << "theta_t acos:  " << theta_t << std::endl << std::endl;
		
		theta_t = asinf((right_dist / center_dist) * sinf(theta_n));

		std::cout << std::endl << "theta_t asin:  " << theta_t << std::endl << std::endl;
		//std::cout << "tt= " << theta_t << std::endl;
		theta_1 = M_PI_2 - theta_t;
		
		//std::cout << "left coord= " << leftEdgeCoord << std::endl;
		//std::cout << "right coord= " << rightEdgeCoord << std::endl;
		
		x = (leftEdgeCoord + rightEdgeCoord)*0.5 - y;
		theta_b = atanf(x/y*tanf(theta_c));
		
		std::cout << "x= " << x << std::endl;
		//std::cout << "theta_b= " << theta_b << std::endl;
		
		theta_2 = theta_1 + theta_b;
	
	
}
