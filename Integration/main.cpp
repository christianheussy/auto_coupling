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
#include <sl/Camera.hpp>
#include "Computer_Vision.h"

//Path
#include "path.h"

//Load constants
#include "config.h"

//Communication
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <iomanip>

// CAN
#include "can.h"

//Boost
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include <boost/accumulators/statistics/stats.hpp>

using namespace cv;
using namespace sl;
using namespace std;
using namespace std::chrono;


// Track bar used for camera calibration
int thresh = 30;
int blur_val = 1;
/*
int thresh_slider_pos = 30;
int blur_slider_pos = 1;

void onThreshbarSlide(int pos)
{
	thresh = pos;
}
	
void onBlurbarSlide(int pos2)
{
	blur_val = pos2;
}
*/

int main(int argc, char** argv)
{

    //Connect to Raspberry Pi
    int sockfd = 0,n = 0, i = 0, kp_flag = 0;
    char recvBuff[800];
    struct sockaddr_in serv_addr;
    float dis_LID, height_LID, closest, t1_LID, t2_LID, choice;
    float delay = 0.0;
    string mess;
    stringstream iss;
    iss.str("");
    iss.clear();

    memset(recvBuff, '0' ,sizeof(recvBuff));
    //int coup_flag;

    while((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
    sockfd = 0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);
    serv_addr.sin_addr.s_addr = inet_addr("10.0.0.20");

    while(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
      close(sockfd);
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_port = htons(5000);
      serv_addr.sin_addr.s_addr = inet_addr("10.0.0.20");
    }

    //Initialize Constants
    config();

    //Send Constants to PI
    while(1)
    {
        n = (read(sockfd, recvBuff, sizeof(recvBuff)));
        recvBuff[n] = 0;
        mess = recvBuff;
        if(mess == "give")
        {
            ifstream myfile;
            myfile.open("configer");
            while(getline(myfile,mess,'\0'))
            {
                strcpy(recvBuff,mess.c_str());
                write(sockfd,recvBuff,sizeof(recvBuff));
                //cout << mess.c_str() << endl;
            }
            myfile.close();
            break;
        }
    }
    //Receive echo of constants
    if(DEBUG == 1){
        cout << "Constants:" << endl;
        n = read(sockfd, recvBuff, sizeof(recvBuff));
        recvBuff[n] = 0;
        iss.str(recvBuff);
        cout << iss.str() << endl;
        iss.str("");
        iss.clear();
    }

    // Launch CAN THREADS
    canInitializeLibrary(); //Initialize Kvaser driver
    std::thread t1(Steering); // Start thread for steering control
    t1.detach();
    std::thread t2(Transmission); // Start thread for transmission control
    t2.detach();
    std::thread t3(Brakes);  // Start thread for brakes
	t3.detach();
    std::thread t4(Reader);  // Start thread to read general CAN signals
	t4.detach();

	//FOR TESTING ONLY
	//std::string Coupling = "/media/ubuntu/SDCARD/Indoor_testing_3_20_4.svo";
	ofstream mystream;
	mystream.open("/home/ubuntu/Documents/SeniorProject/remotetrucks/Camera/Camera_TestData/Angled1.txt");

	// Init time stamp 1
	high_resolution_clock::time_point init_t1 = high_resolution_clock::now();

	// Initialize ZED color stream in HD and depth in QUALITY mode
	sl::Camera zed; //add a filepath here to run code from recorded video
	sl::InitParameters params;
	params.depth_mode = sl::DEPTH_MODE_QUALITY;
	params.sdk_gpu_id = -1;
	params.sdk_verbose = true;
	params.coordinate_units = sl::UNIT_METER;
	params.camera_resolution = sl::RESOLUTION_HD720;
	params.camera_fps = 15;

	// Quit if an error occurred
	sl::ERROR_CODE err = zed.open(params);
	if (err != sl::ERROR_CODE::SUCCESS) {
		std::cout << "Unable to init the ZED:" << errorCode2str(err) << std::endl;
		zed.close();
		return 1;
	}

	// Set the Depth Clamp to an appropriate value
	int depth_clamp = 20000;
	zed.setDepthMaxRangeValue(depth_clamp);

	// Initialize image size (height and width)
	// create empty openCV mats to contain ZED camera images
	// ZED camera data will be pulled into a ZED mat data type then converted to openCV mat
	int width = zed.getResolution().width;
	int height = zed.getResolution().height;
	sl::Mat left(zed.getResolution(), sl::MAT_TYPE_8U_C4);
	cv::Mat left_image(height, width, CV_8UC4, left.getPtr<sl::uchar1>(sl::MEM_CPU));

	// Init time stamp 2
	high_resolution_clock::time_point init_t2 = high_resolution_clock::now();
	auto init_duration = duration_cast<milliseconds>( init_t2 - init_t1 ).count();
	std::cout << "Init duration: " << init_duration << "msec" << endl;

	// crosshairs (xHair, yHair) are used for initial trailer selection and continuous tracking along the path
	int xHair = left_image.cols / 2;
	int yHair = left_image.rows / 2;
	
	// pixel shift parameter to ensure we are on the trailer face
	int pixel_shift = 3;
	
	// auto park enable
	bool start = true;

	// declare path constants
	float limit = 0.0;
	i = 0;
	int recalc = 0;
	float a, b, x_cam , y_cam , x_fwheel , y_fwheel , dist_grad, y_cam_path , y_fwheel_path ;
	
	/*
	// Track bar
	cvNamedWindow("Thresh Trackbar", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Blur Trackbar", CV_WINDOW_AUTOSIZE);
	cvCreateTrackbar("thresh", "Thresh Trackbar", &thresh_slider_pos, 100, onThreshbarSlide);
	cvCreateTrackbar("blur", "Blur Trackbar", &blur_slider_pos, 10, onBlurbarSlide);
	*/
	
	for (;;)
	{
		sl::ERROR_CODE err = zed.grab(sl::SENSING_MODE::SENSING_MODE_STANDARD);
		if (!err)
		{
			// create a ZED Mat to house the image from the left camera (left), then convert to openCV Mat
			err = zed.retrieveImage(left, sl::VIEW_LEFT, sl::MEM_CPU);
		} else if (err == sl::ERROR_CODE::ERROR_CODE_NOT_A_NEW_FRAME) {
			cout << "Warning: got same frame from zed.grab as last time" << endl;
			continue;
		} else {
			cout << "zed.grab failed. Error Code: " << errorCode2str(err) << std::endl;
			cout << err << endl;
			return 1;
		}

		if (left_image.empty()) break; // end of video stream

        if (adjustCrosshairsByInput(xHair, yHair, left_image.rows, left_image.cols)){
            cout << "Press Brake and Shift into Drive" << endl;
			system_enable = 1;
			break;
        }

		drawCrosshairsInMat(left_image, xHair, yHair);
		imshow("TRUCKS", left_image);
		cvWaitKey(10);
	}

	// Accumulation variable for L1, L2, and time delay
	boost::accumulators::accumulator_set<float, boost::accumulators::stats<boost::accumulators::tag::rolling_mean> > left_avg(boost::accumulators::tag::rolling_window::window_size = 10);
	boost::accumulators::accumulator_set<float, boost::accumulators::stats<boost::accumulators::tag::rolling_mean> > right_avg(boost::accumulators::tag::rolling_window::window_size = 10);
	boost::accumulators::accumulator_set<float, boost::accumulators::stats<boost::accumulators::tag::rolling_mean> > delay_avg(boost::accumulators::tag::rolling_window::window_size = 5);
	
	// set initial time delay assuming a loop time of 110ms
	delay_avg(110);
	delay = boost::accumulators::rolling_mean(delay_avg);
	
	for (;;)
	{
		// For loop time stamp 1
		high_resolution_clock::time_point for_t1 = high_resolution_clock::now();

		err = zed.grab(sl::SENSING_MODE::SENSING_MODE_STANDARD);
		if (!err)
		{
			// create a ZED Mat to house the image from the left camera (left), then convert to openCV Mat
			err = zed.retrieveImage(left, sl::VIEW_LEFT, sl::MEM_CPU);
		} else if (err == sl::ERROR_CODE::ERROR_CODE_NOT_A_NEW_FRAME) {
			cout << "Warning: got same frame from zed.grab as last time" << endl;
			continue;
		} else {
			cout << "Returning because zed.grab failed: " << errorCode2str(err) << std::endl;
			return 1;
		}

		if (left_image.empty()) break; // end of video stream

		//edge amd contour detection
		cv::Mat l_contours;
		
		detectEdgesAndContours(left_image, l_contours, height, width, thresh, blur_val);
		namedWindow( "Left Contours", 1);
		imshow( "Left Contours", l_contours);
		cvWaitKey(10);

		int leftedge;	// left edge of trailer
		int rightedge;	// right edge of trailer
		int x_center;	// horizontal center between left and right trailer edges
		int y_center;	// vertical center between top and bottom trailer edges
		
		coordinateGrab(l_contours, xHair, yHair, leftedge, rightedge, x_center, y_center);

		// adjusted coordinate values for the right and left trailer edges
		// this ensures that the distance is grabbed from the front face of the trailer and not the sides
		leftedge += pixel_shift;
		rightedge -= pixel_shift;

		// create a ZED Mat to house the depth map values
		sl::Mat distancevalue;
		err = zed.retrieveMeasure(distancevalue, sl::MEASURE::MEASURE_XYZ, sl::MEM_CPU);
		// distance in m to the left and right edges of the trailer
		float l1;		
		float l2;
		
		distanceGrab(l1, l2, leftedge, rightedge, y_center, distancevalue);

		// increase pixel shift as we get closer
		if(min(l1, l2) < 10)
			pixel_shift = 4;
		else if(min(l1,l2) < 7)
			pixel_shift = 5;
			
		// parameters needed to compute path
		float center_dist;
		float theta_1;
		float theta_2;
		
		// rolling mean of l1 and l2 values
		if(l1 > 0.0 && l1 < 20.0)
			left_avg(l1);
		if(l2 > 0.0 && l2 < 20.0)
			right_avg(l2);
		
		//L1 mean and L2 mean
		float left_mean;
		float right_mean;	
		
		left_mean = boost::accumulators::rolling_mean(left_avg);
		right_mean = boost::accumulators::rolling_mean(right_avg);

		pathInputCalculations_Camera(left_mean, right_mean, center_dist, theta_1, theta_2, leftedge, rightedge);
	   
		if(center_dist < 2.0){
			thresh = 70;
			blur_val = 2;
		}
		else if(center_dist < 2.7){
			thresh = 40;
			blur_val = 2;
		}
		else if(center_dist < 4.0){
			thresh = 43;
			blur_val = 1;
		}
		else{
			thresh = 30;
			blur_val = 1;
		}
		cout << "thresh = " << thresh << endl;
		cout << "blur = " << blur_val << endl;
	   
	   	xHair = x_center;
		yHair = y_center;
		if(max(left_mean, right_mean) < 20)
			depth_clamp = 1000*max(l1, l2) + 2000;
			zed.setDepthMaxRangeValue(depth_clamp);
		

		drawCrosshairsInMat(left_image, xHair, yHair);
		imshow("TRUCKS", left_image);
		// "Why cvWaitKey?"
		// http://stackoverflow.com/questions/5217519/what-does-opencvs-cvwaitkey-function-do
		cvWaitKey(10);

		//Send Selection to Raspberry Pi
		if(SIMPLE == 0){
			choice = theta_1 + theta_2;
			write(sockfd,&choice,sizeof(recvBuff));
		}
		//Read LIDAR distances
	   // if(SIMPLE == 1 || abs(choice) < VIA/2.0)
			
		write(sockfd,"data",strlen("data"));
		n = read(sockfd, recvBuff, sizeof(recvBuff));
		recvBuff[n] = 0;
		iss.str(recvBuff);
		iss >> dis_LID >> t1_LID >> t2_LID >> kp_flag >> height_LID >> closest;
		if(DEBUG == 1)
			cout << endl << endl << iss.str() << endl << endl;
		//   iss >> coup_flag;
		iss.str("");
		iss.clear();

		if(DEBUG > 0){
		std::cout << setw(10) << std::left  << " " <<  setw(15) << std::left    << "Camera"    << "|     " << "LIDAR" << std::endl
			 << setw(10) << std::right << "d:  "  << setw(15) << std::left << center_dist << "|     " << dis_LID << std::endl
			 << setw(10) << std::right << "t1:  " << setw(15) << std::left << theta_1     << "|     " << t1_LID << std::endl
			 << setw(10) << std::right << "t2:  " << setw(15) << std::left << theta_2     << "|     " << t2_LID << std::endl
			 << setw(10) << std::right << "L1 mean:  " << setw(15) << std::left << left_mean   << "height: " << height_LID <<  std::endl
			 << setw(10) << std::right << "L2 mean:  " << setw(15) << std::left << right_mean  << "closest: " << closest <<  std::endl;
		}
		int possible_path;
		float chan_f;
        float theta_path;
        float xdis;
		float shift_center = 0;
		float shift_t1 = 0;
        
        if(LID_ONLY == 1){
            center_dist = dis_LID;
            theta_1 = t1_LID;
            theta_2 = t2_LID;
        }
        
        // Shift the origin by AX_SHIFT in x direction
        if (AX_SHIFT > 0){
            
            shift_center = sqrt(pow(AX_SHIFT, 2) + pow(center_dist, 2) - 2 * AX_SHIFT*center_dist*cosf(theta_1)); // calculated new center_dist based on shift
            
            shift_t1 = acosf((pow(center_dist, 2) - pow(AX_SHIFT, 2) - pow(shift_center, 2)) / (-2 * AX_SHIFT*shift_center)); // calculated new theta_1
            
            center_dist = shift_center; // replacing center_dist with updated value
            
            theta_1 = (acosf(-1) - shift_t1) * (1-2*(theta_1< 0)); // if theta_1 was positive, new theta_1 is positive, else negative, acosf(-1) = pi
        }
        
        if (center_dist <= 0)
            braking_active = 1;
		
		recalc = (abs(y_fwheel_path - y_fwheel) < limit); //checks if we need to recalculate
		if (recalc)
			i++; //iterate so we don't recalculate until we are surely off path
		else
			i = 0; //reset iterator if we are on path
		
		//if (abs(y_fwheel_path - y_fwheel) < limit || path(a, b, center_dist, theta_1, theta_2)){
		if (i < 5 || path(a, b, center_dist, theta_1, theta_2)){
            
            // Steering Calculation
            x_cam = center_dist*cosf(theta_1);  // Camera x coord.
            y_cam = center_dist*sinf(theta_1);  // Camera y coord.
            
			limit = x_cam / 7.0 + .5; // Limit used to trigger path recalc.
			//limit = 1;

            x_fwheel = x_cam - L*cosf(theta_2); // Fifth wheel x coord.
            y_fwheel = y_cam - L*sinf(theta_2); // Fifth wheel y coord.
            
            y_cam_path 	= (a*pow(x_cam, 2) + b*pow(x_cam, 3))*(y_cam > 0);         // Camera path y coord.
            
            y_fwheel_path = (a*pow(x_fwheel, 2) + b*pow(x_fwheel, 3))*(y_fwheel > 0); // Fifth wheel path y coord.
            
            dist_grad  = ((float)SPEED/3600)*(1000/delay);
            

            xdis = sqrt(L*L-pow((y_cam_path - y_fwheel_path),2));  // x distance between ycam and fifth wheel
            
            theta_path = atanf((y_cam_path - y_fwheel_path)/xdis);    // angle of path

            
            chan_f = ((RMIN/dist_grad)*(theta_path - theta_2));      // Difference * constant
            
            if(chan_f > 1) // Max input is 24000
                {
                chan_f = 1;
                }
                      
            if(chan_f < -1) // Min input is -24000
                {
                chan_f = -1;
                }
            
            steering_command = 24000*pow(abs(chan_f),STEER)*(1-2*(chan_f < 0));
				
			possible_path = 1;
		}else{
			cout << "*******************Impossible path********************" << endl;
			possible_path = 0;
			// braking_active = 1;
		}

		if (start)
		{
		// Prompt user ==
		speed_command = 500; // Set speed to .5kph and begin to drive straight back
		start = false;
		}

		// For loop time stamp 2
		high_resolution_clock::time_point for_t2 = high_resolution_clock::now();
		//auto forloop_duration = duration_cast<milliseconds>( for_t2 - for_t1 ).count();
		float forloop_duration = duration_cast<milliseconds>( for_t2 - for_t1 ).count();
		if(brake_pedal == 0){
		delay_avg(forloop_duration);
		delay = boost::accumulators::rolling_mean(delay_avg);
		//std::cout << "For loop duration: " << forloop_duration << "msec" << endl;
		}
		
		// FOR TESTING ONLY
		mystream  << l1 << ","
				 << left_mean << ","
				 << l2 << ","
				 << right_mean << ","
				 << center_dist << ","
				 << theta_1 << ","
				 << theta_2 << ","
				 << a << ","
				 << b << ","
				 << steering_command << ","
				 << possible_path << ","
				 << dis_LID << ","
				 << t1_LID << ","
				 << t2_LID << ","
				 << kp_flag << ","
				 << leftedge << ","
				 << rightedge << ","
				 << theta_path << std::endl;
		
	}
	// FOR TESING ONLY
	mystream.close();
	

	zed.close();

	return 0;
}







