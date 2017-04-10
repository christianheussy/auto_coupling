/**********************************
 **Using ZED with OpenCV
 **********************************/

// Standard
#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// ZED
#include <zed/Camera.hpp>
#include "Computer_Vision.h"

//Path
#include "path.h"

// CAN
#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>
#include "canlib.h"
#include <stdio.h>
#include <atomic>

// Variables for steering thread
static std::atomic<int> steering_command{0}; // Command value (range depends on mode)
static std::atomic<int> steering_mode{1};    // Steering Mode

// Variables for speed thread
static std::atomic<int> direction{0};        // 0 is reverse, 1 is forwards
static std::atomic<int> speed_command{0};    // 1 bit = .001 kph
static std::atomic<int> auto_park_enable{0}; // Activate when driver has foot on brake and shifts into gear

static std::atomic<int> braking_active{0};   // Flag to tell transmission brakes are being externally applied

static std::atomic<int> exit_flag{0};

canHandle hnd1, hnd2, hnd3, hnd4, hnd5; // Declare CanLib Handles and Status
canStatus stat;

unsigned char * steer_data = new unsigned char[8];
unsigned char * speed_data = new unsigned char[3];
unsigned char * brake_data = new unsigned char[8];

using namespace cv;
using namespace std;
using namespace std::chrono;

// path global variables
float RES = 1000.0;
float RMIN = 7.2;
float L = 2.0;


int CheckStat(canStatus stat)
{
      char buf[100];
      if (stat != canOK) {
        buf[0] = '\0';
        canGetErrorText(stat, buf, sizeof(buf));
        printf("Failed, stat=%d (%s)\n", (int)stat, buf);
        //exit(1);
        return 0;
      }
}

// forward declaration for CAN threads
void Steering() {
    
    int message_count{}, checksum_temp{}, checksum_calc{};
    
    long steering_command_ID = 0x18FFEF27;
    unsigned int steering_command_DLC = 8;            //Data length
    unsigned int steering_command_FLAG = canMSG_EXT;  //Indicates extended ID
    
    hnd1 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);        // Open channel for Steer thread
    stat=canSetBusParams(hnd1, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
    CheckStat(stat);
    stat=canSetBusOutputControl(hnd1, canDRIVER_NORMAL);        //Set driver type normal
    CheckStat(stat);
    stat=canBusOn(hnd1);                                        //Take channel on bus
    CheckStat(stat);
    
    while(exit_flag!=true)
    {
        message_count++;
        if (message_count > 15)
        {
            message_count = 0;
        }
        steer_data[0] = steering_mode;
        steer_data[1] =  (steering_command & 0x000000FF);
        steer_data[2] = ((steering_command & 0x0000FF00) >> 8);
        steer_data[3] = ((steering_command & 0x00FF0000) >> 16);
        steer_data[4] = ((steering_command & 0xFF000000) >> 24);
        steer_data[5] = 0xFF;
        steer_data[6] = 0xFF;
        
        //Check Sum Calculation
        checksum_temp = steer_data[0] + steer_data[1] + steer_data[2] +
        steer_data[3] + steer_data[4] + steer_data[5] + steer_data[6] +
        (steering_command_ID  & 0x000000FF) +
        ((steering_command_ID & 0x0000FF00) >> 8)  +
        ((steering_command_ID & 0x00FF0000) >> 16) +
        ((steering_command_ID & 0xFF000000) >> 24) +
        (message_count);
        
        checksum_calc = ((checksum_temp >> 4) + checksum_temp) & 0x000F;
        
        steer_data[7] =  (checksum_calc << 4) + (message_count); // put checksum into last byte
        
        stat=canWriteWait(hnd1, steering_command_ID, steer_data, steering_command_DLC, steering_command_FLAG,50);
        //CheckStat(stat);
        
        this_thread::yield();
        this_thread::sleep_for (chrono::milliseconds(10));
        
    }
    
    stat = canBusOff(hnd1); // Take channel offline
    CheckStat(stat);
    canClose(hnd1);
}


void Transmission() {// Thread used to control the speed using the transmission
    
    long Drive_ID = 0x18FF552B;
    unsigned int Drive_DL = 8;
    unsigned int Drive_FLAG = canMSG_EXT;
    hnd2 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);        // Open channel for speed control
    stat=canSetBusParams(hnd2, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
    CheckStat(stat);
    stat=canSetBusOutputControl(hnd2, canDRIVER_NORMAL);        // set driver type normal
    CheckStat(stat);
    stat=canBusOn(hnd2);                                        // take channel on bus and start reading messages
    CheckStat(stat);
    while (true)
    {

            speed_data[0] = ((speed_command & 0x3F) << 2) + auto_park_enable;
            speed_data[1] = ((speed_command & 0x3FC0) >> 6);
            speed_data[2] = (braking_active << 4) + (direction << 2) + ((speed_command & 0xC000) >> 14);
            speed_data[3] = 0xFF;
            speed_data[4] = 0xFF;
            speed_data[5] = 0xFF;
            speed_data[6] = 0xFF;
            speed_data[7] = 0xFF;
            stat=canWrite(hnd2, Drive_ID, speed_data, Drive_DL, Drive_FLAG);
            //CheckStat(stat);
        
        this_thread::yield();
        this_thread::sleep_for (chrono::milliseconds(10));
        
        if (exit_flag == 1)
            break;
        
    }
    stat = canBusOff(hnd2); // Take channel offline
    CheckStat(stat);
    canClose(hnd2);
}

void Brakes() {//Thread to Apply Brakes
    
    hnd5 = canOpenChannel(1, canOPEN_REQUIRE_EXTENDED);        // Open channel for speed control
    stat=canSetBusParams(hnd5, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
    CheckStat(stat);
    stat=canSetBusOutputControl(hnd5, canDRIVER_NORMAL);        // set driver type normal
    CheckStat(stat);
    stat=canBusOn(hnd5);                                        // take channel on bus and start reading messages
    CheckStat(stat);
    
    
    int brake_pressure_value = 15; // 8 bar
    int brake_pressure_command;
    
    brake_pressure_command = (brake_pressure_value & 0x000000FF);
    long Brake_ID = 0x750;
    unsigned int Brake_DL = 8; //3 Bytes ??
    unsigned int Brake_FLAG = {}; //Indicates extended ID
    
    
    while (true)
    {
        brake_data[0] = brake_pressure_command; //Front Left
        brake_data[1] = brake_pressure_command; //Front Right
        brake_data[2] = brake_pressure_command; //Rear Left
        brake_data[3] = brake_pressure_command; //Rear Right
        brake_data[4] = 0;
        //set bits 6.1 and 6.4 to 1
        
        
        if(braking_active == 1)
        {
            brake_data[5] = ((0xF & 0x9) );
        }
        else if (braking_active == 0){
            brake_data[5] = 0;
        }
        
        stat = canWrite(hnd5, Brake_ID, brake_data, Brake_DL, {});
        this_thread::yield();
        this_thread::sleep_for (chrono::milliseconds(10));
        
        if (exit_flag == 1)
            break;
        
    }
    stat = canBusOff(hnd5); // Take channel offline
    CheckStat(stat);
    canClose(hnd5);
}


int main(int argc, char** argv)
{
    
    // Launch CAN THREADS
    canInitializeLibrary(); //Initialize driver
    
    std::thread t1(Steering); // Start thread for steering control
    std::thread t2(Transmission); // Start thread for transmission control
    std::thread t3(Brakes);  // Start thread to read
    
    
	/* FOR TESTING ONLY
	std::string Coupling = "/media/ubuntu/SDCARD/Indoor_testing_3_20_4.svo";
	ofstream mystream;
	mystream.open("/home/ubuntu/Documents/SeniorProject/remotetrucks/Camera/Camera_TestData/IndoorTestData4.txt");
	*/
	
	// Init time stamp 1
	high_resolution_clock::time_point init_t1 = high_resolution_clock::now();
	
	// Initialize ZED color stream in HD and depth in QUALITY mode
	sl::zed::Camera* zed = new sl::zed::Camera(sl::zed::VGA); //add a filepath here to run code from recorded video
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
	
	// Set the Depth Clamp to an appropriate value 
	int depth_clamp = 20000;																	
	zed->setDepthClampValue(depth_clamp);														

	// Initialize image size (height and width)
	// create empty openCV mats to contain ZED camera images
	// ZED camera data will be pulled into a ZED mat data type then converted to openCV mat
	int width = zed->getImageSize().width;														
	int height = zed->getImageSize().height;													
	cv::Mat left_image(height, width, CV_8UC4,1);	
	
	// Init time stamp 2
	high_resolution_clock::time_point init_t2 = high_resolution_clock::now();		
	auto init_duration = duration_cast<milliseconds>( init_t2 - init_t1 ).count();		
	std::cout << "Init duration: " << init_duration << "msec" << endl;					

	// crosshairs (xHair, yHair) are used for initial trailer selection and continuous tracking along the path
	int xHair = left_image.cols / 2; 															
	int yHair = left_image.rows / 2;
	 															
	// counter to limit the number of times edge and contour detection are performed
	int count = 0;
	
	// auto park enable
	bool start = true;

	for (;;)
	{		
		if (!zed->grab(sl::zed::SENSING_MODE::FILL))
		{
			// create a ZED Mat to house the image from the left camera (left), then convert to openCV Mat
			sl::zed::Mat left = zed->retrieveImage(sl::zed::SIDE::LEFT);					
			sl::zed::slMat2cvMat(left).copyTo(left_image);										
		} else if (err == sl::zed::ERRCODE::NO_NEW_FRAME) {
			cout << "Warning: got same frame from zed->grab as last time" << endl;
			continue;
		} else {
			cout << "Returning because zed->grab failed: " << errcode2str(err) << std::endl;
			return 1;
		}
		
		if (left_image.empty()) break; // end of video stream
		
		if (adjustCrosshairsByInput(xHair, yHair, left_image.rows, left_image.cols))
			break;
		drawCrosshairsInMat(left_image, xHair, yHair);
		imshow("this is you, smile! :)", left_image);
		
	}

	for (;;) 
	{
		// For loop time stamp 1
		high_resolution_clock::time_point for_t1 = high_resolution_clock::now();
	
		sl::zed::ERRCODE err = zed->grab(sl::zed::SENSING_MODE::FILL);
		if (!err)
		{
			// create a ZED Mat to house the image from the left camera (left), then convert to openCV Mat
			sl::zed::Mat left = zed->retrieveImage(sl::zed::SIDE::LEFT);						
			sl::zed::slMat2cvMat(left).copyTo(left_image);										
		} else if (err == sl::zed::ERRCODE::NO_NEW_FRAME) {
			cout << "Warning: got same frame from zed->grab as last time" << endl;
			continue;
		} else {
			cout << "Returning because zed->grab failed: " << errcode2str(err) << std::endl;
			return 1;
		}

		if (left_image.empty()) break; // end of video stream

		int thresh = 30;
		int blur = 1;
				
		//count so this section doesn't happen every time
		count++;
		if (count == 1) {
			count = 0;
			
			//edge detection left image
			Mat left_edges(height, width, CV_8UC4,1);
			cvtColor(left_image, left_edges, COLOR_BGR2GRAY);
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
			//cvWaitKey(30)
			
			// create a ZED Mat to house the depth map values
			sl::zed::Mat distancevalue;
			distancevalue = zed->retrieveMeasure(sl::zed::MEASURE::DEPTH);
			
			int leftedge;	// left edge of trailer 
			int rightedge;	// right edge of trailer 
			int x_center;	// horizontal center between left and right trailer edges
			int y_center;	// vertical center between top and bottom trailer edges
			float l1;		// distance in mm to the left edge of the trailer
			float l2;		// distance in mm to the right edge of the trailer
			
			// adjusted coordinate values for the right and left trailer edges
			// these values are shfted inward by 'pixel_shift'
			int left_coord;  			
			int right_coord;			
			int pixel_shift = 3;		
			
						
			coordinateGrab(l_contours, xHair, yHair, leftedge, rightedge, x_center, y_center);
			
			left_coord = leftedge;
			left_coord += pixel_shift;
			right_coord = rightedge;
			right_coord -= pixel_shift;
			
			distanceGrab(l1, l2, left_coord, right_coord, y_center, distancevalue);
			
			// this ensures that the distance is grabbed from the front face of the trailer and not the sides
			if(min(l1, l2) < 10000)
				pixel_shift = 4;
			else if(min(l1,l2) < 7000)
				pixel_shift = 5;
				
			float l1_meters = l1 / 1000;
			float l2_meters = l2 / 1000;	
			
			/* FOR TESTING ONLY
			mystream << "l1= " << l1_meters << " meters" << std::endl 
					<< "l2= " << l2_meters << " meters" << std::endl << std::endl;
			*/
			
			float center_dist;
			float theta_1;
			float theta_2;
			float range = 2048.0;
          
            
            float limit = 0.0;
            float a, b, x_cam, y_cam, x_fwheel, y_fwheel, dist_grad, y_cam_next, y_fwheel_next, st_coeff;

            
			pathInputCalculations_Camera(l1_meters, l2_meters, center_dist, theta_1, theta_2, left_coord, right_coord);
            st_coeff = range*RMIN/2;
            
            cout << "d = " << center_dist << endl;
			cout << "t1 = " << theta_1 << endl;
			cout << "t2 =" << theta_2 << endl;
			cout << "L1 = " << l1_meters << endl;
			cout << "L2 = " << l2_meters << endl;

            if (abs(y_fwheel_next - y_fwheel) < limit || path(a, b, center_dist, theta_1, theta_2)){
				x_cam = center_dist*cosf(theta_1);
				y_cam = center_dist*sinf(theta_1);
				x_fwheel = x_cam - L*cosf(theta_2);
				y_fwheel = y_cam - L*sinf(theta_2);

				dist_grad = x_fwheel / RES;				// Set distance gradient
				y_cam_next = a*pow(x_cam - dist_grad, 2) + b*pow(x_cam - dist_grad, 3);
				y_fwheel_next = a*pow(x_fwheel - dist_grad, 2) + b*pow(x_fwheel - dist_grad, 3);
				limit = sqrt(x_cam);
				steering_command = st_coeff * (y_cam_next - y_fwheel_next - y_cam + y_fwheel) / dist_grad;
			}else{
				cout << "Impossible path" << endl;
				braking_active = 1;
			}
                
			if (start)
			{
			// Prompt user
            cout << "Please press brake pedal" << endl;
            cout << "Please shift into drive" << endl;
            cout << "Please press waitkey30 to activate Transmission control" << endl;
            cout << "once brake is pressed and drive is selected" << endl;
			system("pause"); // Wait for keypad to continue
            auto_park_enable = 1; // Start transmission control
            
            speed_command = .75; // Set speed to .75kph and begin to drive straight back
            
            start = false;
			}
            
            cout << "we got this far" << endl;

			
			xHair = x_center;
			yHair = y_center;
			if(max(l1, l2) < 20000)
				depth_clamp = max(l1, l2) + 2000;	
			zed->setDepthClampValue(depth_clamp);		
		}

		drawCrosshairsInMat(left_image, xHair, yHair);
		imshow("this is you, smile! :)", left_image);
		// "Why cvWaitKey?"
		// http://stackoverflow.com/questions/5217519/what-does-opencvs-cvwaitkey-function-do
		cvWaitKey(30);
		
		// For loop time stamp 2
		high_resolution_clock::time_point for_t2 = high_resolution_clock::now();
		auto forloop_duration = duration_cast<milliseconds>( for_t2 - for_t1 ).count();	
		std::cout << "For loop duration: " << forloop_duration << "msec" << endl;

	}
	// FOR TESING ONLY
	// mystream.close();
	delete zed;
        
        t1.join(); // Wait for t1 to finish
        t2.join(); // Wait for t2 to finish
        t3.join(); // Wait for t3 to join
        
	return 0;
}

// code for previously declared functions moved to Computer_Vision_impl.cpp





