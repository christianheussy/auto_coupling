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
#include <thread>
#include "canlib.h"
#include <stdio.h>
#include <atomic>

//Boost
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include <boost/accumulators/statistics/stats.hpp>

using namespace cv;
using namespace sl;
using namespace std;
using namespace std::chrono;


// Variables for steering thread
static std::atomic<int> steering_command{0}; // Command value (range depends on mode)
static std::atomic<int> steering_mode{2};    // Steering Mode

// Variables for speed thread
static std::atomic<int> direction{0};        // 0 is reverse, 1 is forwards
static std::atomic<int> speed_command{0};    // 1 bit = .001 kph
static std::atomic<int> auto_park_enable{0}; // Activate when driver has foot on brake and shifts into gear

// Variables for braking thread
static std::atomic<int> braking_active{0};   // Flag to apply brakes and signal brakes are being externally applied

// Variables for suspension thread
static std::atomic<int> requested_height{0};
static std::atomic<int> height_control_enable{0};

// Variables for reading off CAN bus
static std::atomic<int> requested_gear{0};
static std::atomic<int> brake_pedal{0};

static std::atomic<int> system_enable{0};  
static std::atomic<int> exit_flag{0};        // Flag used to signal threads to quit execution

// CAN lib specific variables
canHandle hnd1, hnd2, hnd3, hnd4, hnd5;      // Declare CanLib Handles and Status
canStatus stat;

// Character arrays for CAN data
unsigned char * steer_data = new unsigned char[8];
unsigned char * speed_data = new unsigned char[3];
unsigned char * brake_data = new unsigned char[8];
unsigned char * brake_pedal_data = new unsigned char[8];
unsigned char * current_gear_data = new unsigned char[8];

long brake_pedal_ID  = 0x18F0010B;              // Id for brake signal
long current_gear_ID = 0x18F00503;              // Id for transmission gear signal

// Variables that are used by the CAN read function
unsigned int gear_DLC, brake_pedal_DLC;
unsigned int gear_FLAG, brake_pedal_FLAG;
unsigned long gear_TIME, brake_pedal_TIME;

//L1 mean and L2 mean
float left_mean;
float right_mean;

int CheckStat(canStatus stat){
    char buf[100];
    if (stat != canOK)
    {
        buf[0] = '\0';
        canGetErrorText(stat, buf, sizeof(buf));
        printf("Failed, stat=%d (%s)\n", (int)stat, buf);
        return 0;
    }
}

void Steering()
{
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

    while(true)
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

        if (exit_flag == 1){
            break;
        }

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

        if (exit_flag == 1){
            break;
        }

        this_thread::yield();
        this_thread::sleep_for (chrono::milliseconds(10));

    }
    stat = canBusOff(hnd2); // Take channel offline
    CheckStat(stat);
    canClose(hnd2);
}

void Brakes() {//Thread to Apply Brakes

    hnd3 = canOpenChannel(1, canOPEN_REQUIRE_EXTENDED);         // Open channel for speed control
    stat=canSetBusParams(hnd3, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
    CheckStat(stat);
    stat=canSetBusOutputControl(hnd3, canDRIVER_NORMAL);        // set driver type normal
    CheckStat(stat);
    stat=canBusOn(hnd3);                                        // take channel on bus and start reading messages
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

        if(braking_active == 1)
        {
            brake_data[5] = ((0xF & 0x9) );
        }
        else if (braking_active == 0){
            brake_data[5] = 0;
        }

        stat = canWrite(hnd3, Brake_ID, brake_data, Brake_DL, {});
        this_thread::yield();
        this_thread::sleep_for (chrono::milliseconds(10));

        if (exit_flag == 1){
            break;
            }

    }
    stat = canBusOff(hnd3); // Take channel offline
    CheckStat(stat);
    canClose(hnd3);
}

void Reader(){

    hnd5 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);        // Open channel for reading brake and current trans gear
    stat=canSetBusParams(hnd5, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
    CheckStat(stat);                                            // Check set bus parameters was success
    stat=canSetBusOutputControl(hnd5, canDRIVER_NORMAL);        // set driver type normal
    CheckStat(stat);                                            // Check driver initialized correctly
    stat=canBusOn(hnd5);                                        // take channel on bus and start reading messages
    CheckStat(stat);

    while(true)
    {
    // Read signal containing brake pedal status
    canReadSpecific(hnd5, brake_pedal_ID, brake_pedal_data, &brake_pedal_DLC, &brake_pedal_FLAG, &brake_pedal_TIME);

    // Read signal for transmission requested gear
    canReadSpecific(hnd5, current_gear_ID, current_gear_data, &gear_DLC, &gear_FLAG, &gear_TIME);

    requested_gear = current_gear_data[5];      // Retrieve ASCII character from data 6th byte

    brake_pedal = ((0xC0 & brake_pedal_data[0]) >> 6); // Retrieve two bit brake pedal status from from message

    this_thread::yield();
    this_thread::sleep_for (chrono::milliseconds(100));
    
		if(system_enable == 1 && requested_gear == 68 && brake_pedal == 1)
		{
			auto_park_enable = 1;
		}

        if (exit_flag == 1)
        {break;
        }
    }
    stat = canBusOff(hnd5); // Take channel offline
    CheckStat(stat);
    canClose(hnd5);
}

int main(int argc, char** argv)
{
	
	//Accumulation variable for L1, L2
	boost::accumulators::accumulator_set<float, boost::accumulators::stats<boost::accumulators::tag::rolling_mean> > left_avg(boost::accumulators::tag::rolling_window::window_size = 10);
	boost::accumulators::accumulator_set<float, boost::accumulators::stats<boost::accumulators::tag::rolling_mean> > right_avg(boost::accumulators::tag::rolling_window::window_size = 10);
	boost::accumulators::accumulator_set<float, boost::accumulators::stats<boost::accumulators::tag::rolling_mean> > delay_avg(boost::accumulators::tag::rolling_window::window_size = 5);
   
    //Connect to Raspberry Pi
    int sockfd = 0,n = 0, i = 0, kp_flag = 0;
    char recvBuff[800];
    struct sockaddr_in serv_addr;
    float dis_LID, height_LID, closest, t1_LID, t2_LID, choice;
    float delay = 0;
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
    canInitializeLibrary(); //Initialize driver


    std::thread t1(Steering); // Start thread for steering control
    t1.detach();
    std::thread t2(Transmission); // Start thread for transmission control
    t2.detach();
    std::thread t3(Brakes);  // Start thread to read
	t3.detach();
    std::thread t4(Reader);  // Start thread to read
	t4.detach();

	//FOR TESTING ONLY
	//std::string Coupling = "/media/ubuntu/SDCARD/Indoor_testing_3_20_4.svo";
	ofstream mystream;
	mystream.open("/home/ubuntu/Documents/SeniorProject/remotetrucks/Camera/Camera_TestData/CAN+Camera_Straight_1.txt");

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

	// counter to limit the number of times edge and contour detection are performed
	int count = 0;

	// auto park enable
	bool start = true;

	float limit = 0.0;
	float a, b, x_cam , y_cam , x_fwheel , y_fwheel , dist_grad, y_cam_next , y_fwheel_next ;

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

	delay_avg(120);
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

		int thresh = 30;
		int blur = 1;

		//count so this section doesn't happen every time
		count++;
		if (count == 1) {
			count = 0;

			//edge detection left image
			cv::Mat left_edges(height, width, CV_8UC4,1);
			cvtColor(left_image, left_edges, COLOR_BGR2GRAY);
			GaussianBlur(left_edges, left_edges, Size(7,7), blur, blur);
			Canny(left_edges, left_edges, thresh, thresh*3, 3);

			//contour detection left image
			cv::Mat l_contours = cv::Mat::zeros(left_edges.rows, left_edges.cols, CV_8UC4);
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
			//cvWaitKey(10);

			// create a ZED Mat to house the depth map values
			sl::Mat distancevalue;
			err = zed.retrieveMeasure(distancevalue, sl::MEASURE::MEASURE_XYZ, sl::MEM_CPU);

			int leftedge;	// left edge of trailer
			int rightedge;	// right edge of trailer
			int x_center;	// horizontal center between left and right trailer edges
			int y_center;	// vertical center between top and bottom trailer edges
			float l1;		// distance in m to the left edge of the trailer
			float l2;		// distance in m to the right edge of the trailer

			// adjusted coordinate values for the right and left trailer edges
			// these values are shfted inward by 'pixel_shift'
			int left_coord;
			int right_coord;
			int pixel_shift =3;

			coordinateGrab(l_contours, xHair, yHair, leftedge, rightedge, x_center, y_center);

			left_coord = leftedge;
			left_coord += pixel_shift;
			right_coord = rightedge;
			right_coord -= pixel_shift;

			distanceGrab(l1, l2, left_coord, right_coord, y_center, distancevalue);

			// this ensures that the distance is grabbed from the front face of the trailer and not the sides
			if(min(l1, l2) < 10)
				pixel_shift = 4;
			else if(min(l1,l2) < 7)
				pixel_shift = 5;

			float center_dist;
			float theta_1;
			float theta_2;
			
			
			
			// rolling mean of l1 and l2 values
			if(l1 > 0.0 && l1 < 20.0)
				left_avg(l1);
			if(l2 > 0.0 && l2 < 20.0)
				right_avg(l2);
			
			left_mean = boost::accumulators::rolling_mean(left_avg);
			right_mean = boost::accumulators::rolling_mean(right_avg);

			pathInputCalculations_Camera(left_mean, right_mean, center_dist, theta_1, theta_2, left_coord, right_coord);
           
           
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
            if (abs(y_fwheel_next - y_fwheel) < limit || path(a, b, center_dist, theta_1, theta_2)){
                
                if(LID_ONLY == 1){
                    center_dist = dis_LID;
                    theta_1 = t1_LID;
                    theta_2 = t2_LID;
                }
				x_cam = center_dist*cosf(theta_1);
				y_cam = center_dist*sinf(theta_1);
				x_fwheel = x_cam - L*cosf(theta_2);
				y_fwheel = y_cam - L*sinf(theta_2);
				
				
				
				dist_grad = ((float)SPEED /3600)*(1000/delay);				// Set distance gradient
				y_cam_next = a*pow(x_cam - dist_grad, 2) + b*pow(x_cam - dist_grad, 3);
				y_fwheel_next = a*pow(x_fwheel - dist_grad, 2) + b*pow(x_fwheel - dist_grad, 3);
				limit = x_cam/8.0;
				float xdis = sqrt(L*L-pow((y_cam-y_fwheel),2));
				chan_f = (RMIN*(atanf((y_cam_next - y_fwheel_next)/xdis)-atanf((y_cam - y_fwheel)/xdis))/dist_grad);
				
				//y_cam = y_cam_next;
				//y_fwheel = y_fwheel_next;
				
				if(chan_f > 1)
					chan_f = 1;
				else if(chan_f < -1)
					chan_f = -1;
				
				if(chan_f < 0)
					steering_command = -24000*pow(abs(chan_f),STEER);
				else
					steering_command = 24000*pow(chan_f,STEER);
				
				possible_path = 1;
				cout << "in the loop" << endl;
			}else{
				cout << "*******************Impossible path********************" << endl;
				possible_path = 0;
				// braking_active = 1;
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
					 << kp_flag << std::endl;


			if (start)
			{
			// Prompt user ==

            speed_command = 500; // Set speed to .5kph and begin to drive straight back
	        cout << "in the loop2" << endl;
            start = false;
			}

            //cout << "we got this far" << endl;


			xHair = x_center;
			yHair = y_center;
			if(max(l1, l2) < 20)
				depth_clamp = 1000*max(l1, l2) + 2000;
			//std::cout << "max range= " << zed.getDepthMaxRangeValue() << std::endl;
			//std::cout << "depth_clamp= " << depth_clamp << std::endl;
			zed.setDepthMaxRangeValue(depth_clamp);
		}

		drawCrosshairsInMat(left_image, xHair, yHair);
		imshow("TRUCKS", left_image);
		// "Why cvWaitKey?"
		// http://stackoverflow.com/questions/5217519/what-does-opencvs-cvwaitkey-function-do
		cvWaitKey(10);

		// For loop time stamp 2
		high_resolution_clock::time_point for_t2 = high_resolution_clock::now();
		//auto forloop_duration = duration_cast<milliseconds>( for_t2 - for_t1 ).count();
		float forloop_duration = duration_cast<milliseconds>( for_t2 - for_t1 ).count();
		if(brake_pedal == 0){
		delay_avg(forloop_duration);
		delay = boost::accumulators::rolling_mean(delay_avg);
		//std::cout << "For loop duration: " << forloop_duration << "msec" << endl;
		}
	}
	// FOR TESING ONLY
	mystream.close();
	zed.close();

        /*
        t1.join(); // Wait for t1 to finish
        t2.join(); // Wait for t2 to finish
        t3.join(); // Wait for t3 to join
        */

	return 0;
}

// code for previously declared functions moved to Computer_Vision_impl.cpp





