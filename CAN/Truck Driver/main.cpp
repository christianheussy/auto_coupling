#include <thread>
#include <iostream>
#include <chrono>
#include "canlib.h"
#include <stdio.h>
#include <atomic>
#include <conio.h>

// OpenCV
//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

int CheckStat(canStatus stat);

// Variables for steering thread
static std::atomic<int> steering_command{0}; // Command value (range depends on mode)
static std::atomic<int> steering_mode{2};    // Steering Mode
static std::atomic<int> steering_desired{0};    // Steering Mode

// Variables for speed thread
static std::atomic<int> direction{0};        // 0 is reverse, 1 is forwards
static std::atomic<int> speed_command{0};    // 1 bit = .001 kph
static std::atomic<int> auto_park_enable{0}; // Activate when driver has foot on brake and shifts into gear

// Variables for suspension thread
static std::atomic<int> requested_height{0};
static std::atomic<int> height_control_enable{0};

// Variables read off the CAN bus
static std::atomic<int> requested_gear{0};
static std::atomic<int> brake_pedal{0};

// Variables for braking thread
static std::atomic<int> braking_active{0};   // Flag to tell transmission brakes are being externally applied

static std::atomic<int> exit_flag{0};        // Exit flag to kill CAN threads when program is finished

// Character arrays for can message data
unsigned char * steer_data = new unsigned char[8];
unsigned char * speed_data = new unsigned char[3];
unsigned char * brake_data = new unsigned char[8];

canHandle hnd1, hnd2, hnd3, hnd4, hnd5; // Declare CanLib Handles and Status
canStatus stat;

void Steering(){
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
    int brake_pressure_value = 12; // 8 bar
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
        if (exit_flag == 1)
        {
        break;
        }
    }
    stat = canBusOff(hnd3); // Take channel offline
    CheckStat(stat);
    canClose(hnd3);
}

void Suspension(){
hnd4 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);
stat=canSetBusParams(hnd4, canBITRATE_250K, 0, 0, 0, 0, 0);
stat=canSetBusOutputControl(hnd4, canDRIVER_NORMAL);
stat=canBusOn(hnd4);
CheckStat(stat);

int command = 0;

// Create ASC6 initial level command message
long ASC6_ID = 0x18D12F27;
unsigned char * ASC6_DATA = new unsigned char[8];
unsigned int ASC6_DLC = 8;
unsigned int ASC6_FLAG = canMSG_EXT;

// ASC2 command message w/ nominal level request axle set to preset level
long ASC2_ID = 0xCD22f2b;
unsigned char * ASC2_DATA = new unsigned char[8];
unsigned int ASC2_DLC = 8; //Data length
unsigned int ASC2_FLAG = canMSG_EXT; //Indicates extended ID
unsigned long ASC2_TIMEOUT = 1000; // Timeout for read wait

// Create ASC1 status message
long ASC1_ID = 0x18FE5A27;
unsigned char * ASC1_DATA = new unsigned char[8];
unsigned int * ASC1_DLC; //Data length
unsigned int * ASC1_FLAG;
unsigned long * ASC1_TIME; //Indicates extended ID

// ASC3 status message
long ASC3_ID = 0x18FE5927;
unsigned char * ASC3_DATA = new unsigned char[8];
unsigned int * ASC3_DLC; //Data length
unsigned int * ASC3_FLAG; //Indicates extended ID
unsigned long * ASC3_TIME; // Timeout for read wait

// read current from angle from ASC3
// stat=canReadSpecific(hnd4, ASC3_ID, ASC3_DATA, ASC3_DLC, ASC3_FLAG, ASC3_TIME);
// CheckStat(stat);

ASC2_DATA[0] = 0;
ASC2_DATA[1] = (1 << 4); //message ASC2 set to preset level

    while(true){

    command = requested_height + 32000; //Set requested height to command value

    ASC6_DATA[4] =  (command & 0x000000FF);
    ASC6_DATA[5] = ((command & 0x0000FF00) >> 8);

    stat = canWrite(hnd4, ASC6_ID, ASC6_DATA, ASC6_DLC, ASC6_FLAG);

        if (height_control_enable = 1)
        {
            stat = canWrite(hnd3, ASC2_ID, ASC2_DATA, ASC2_DLC, ASC2_FLAG);
        }

        if (exit_flag == 1)
        {
        break;
        }

    this_thread::yield();
    this_thread::sleep_for (chrono::milliseconds(100));

    }

stat = canBusOff(hnd4); // Take channel offline
CheckStat(stat);
canClose(hnd4);
  }


/*
void smoother()
{
    int increment = 100;
    int limit = 0;
    int counter = 0;

    while(true){

    error = steering_desired - steering_command;


    time = 100;



                steering_command = steering_command + increment;
                cout << "steering command=" << steering_command << endl;
                this_thread::yield();
                this_thread::sleep_for (chrono::milliseconds(500/limit));


                //steering_command = steering_command - increment;
                //cout << "steering command=" << steering_command << endl;
                this_thread::yield();
                this_thread::sleep_for (chrono::milliseconds(500/abs(limit)));

            if (exit_flag == 1){
            break;
            }
    }

}
*/


void Reader(){

    unsigned char * brake_pedal_data = new unsigned char[8];
    unsigned char * current_gear_data = new unsigned char[8];

    long brake_pedal_ID  = 0x18F0010B;              // Id for brake signal
    long current_gear_ID = 0x18F00503;              // Id for transmission gear signal

    // Variables that are used by the CAN read function
    unsigned int gear_DLC, brake_pedal_DLC;
    unsigned int gear_FLAG, brake_pedal_FLAG;
    unsigned long gear_TIME, brake_pedal_TIME;
    // Variables to store brake pedal and gear status
    int brake_pedal = -5;
    int requested_gear = 0;

    hnd5 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);        // Open channel for reading brake and current trans gear
    stat=canSetBusParams(hnd5, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
    CheckStat(stat);                                            // Check set bus parameters was success
    stat=canSetBusOutputControl(hnd5, canDRIVER_NORMAL);        // set driver type normal
    CheckStat(stat);                                            // Check driver initialized correctly
    stat=canBusOn(hnd5);                                        // take channel on bus and start reading messages
    CheckStat(stat);

    while(true)
    {

    canReadSpecific(hnd5, brake_pedal_ID, brake_pedal_data, &brake_pedal_DLC, &brake_pedal_FLAG, &brake_pedal_TIME);
                // Read transmission requested gear signal
    canReadSpecific(hnd5, current_gear_ID, current_gear_data, &gear_DLC, &gear_FLAG, &gear_TIME);

                // Retrieve ASCII character from data 6th byte
    requested_gear = current_gear_data[5];
                // Retrieve brake pedal from status from message
    brake_pedal = (0x02 & brake_pedal_data[0]);

    this_thread::yield();
    this_thread::sleep_for (chrono::milliseconds(50));

        if (exit_flag == 1)
        {
        break;
        }
    }
    stat = canBusOff(hnd5); // Take channel offline
    CheckStat(stat);
    canClose(hnd5);
}



int main() {

    canInitializeLibrary(); //Initialize driver

    int Choice;
    int steeringGain = 100;


    std::thread t1 (Steering); // Start thread for steering control
    std::thread t2 (Transmission); // Start thread for transmission control
    std::thread t3 (Brakes);  // Start thread to read
    std::thread t4 (Suspension);
    std::thread t5 (Reader);

    int c=0;
    do{
        Choice = getch();
        switch(Choice) { // the real value

        case 72: //Arrow Up
            speed_command = speed_command + 500;
            cout << "Speed =" << speed_command*.001 << "kph" << endl;
            break;

        case 80: //Arrow Down
            if (speed_command != 0)
            {
            speed_command = speed_command - 500;
            }
            cout << "Speed =" << speed_command*.001 << "kph" << endl;
            break;

        case 77: //Right Arrow`
            if (steering_command != 9000)
            steering_command = steering_command + steeringGain*10;
            cout <<  "Steering Desired= " << steering_command << endl;
            break;

        case 75: //Left Arrow
            if (steering_command != -9000)
            steering_command = steering_command - steeringGain*10;
            cout <<  "Steering Desired= " << steering_command << endl;
            break;

        case 122: //Z Big Left

            steering_command = steering_command - steeringGain*10;
            cout <<  "Steering Desired= " << steering_command << endl;
            break;

        case 120: //X Big Right
            steering_command = steering_command + steeringGain*10;
            cout <<  "Steering Desired= " << steering_command << endl;
            break;

        case 32:
            if (braking_active == 1){
                braking_active = 0;
                cout << "Brakes Off" << endl;
            }
            else if (braking_active == 0){
                braking_active = 1;
                cout << "Brakes On" << endl;
            }
            break;

        case 100:
            if (direction == 0){
                direction = 1;
                cout << "forward" << endl;
            }
            else if (direction == 1){
                direction = 0;
                cout << "reverse" << endl;
            }
            break;

        case 101:
            auto_park_enable = 1;
            height_control_enable = 1;
            cout << "AUTOBOTS ROLL OUT" << endl;
            break;

        case 46:
            requested_height = requested_height + 100;
            cout << "Requested Height:"<< requested_height << endl;
            break;

        case 44:
            requested_height = requested_height - 100;
            cout << "Requested Height:"<< requested_height << endl;
            break;

        case 27: //Exit Key
            c++;
            exit_flag = true;
            break;

        // R = 114
        case 114:
        if (requested_gear == 68)
        {
        cout << "In Drive" << endl;
        }
        else if (requested_gear == 78)
        {
        cout << "In Neutral" << endl;
        }
        else{
        cout << "Requested Gear Val:" << requested_gear << endl;
        }

        if ( brake_pedal == 1)
        cout << "Brake Pedal Pressed" << endl;

        if (brake_pedal == 0)
        cout << "Brake Pedal Not Pressed" << endl;

        break;

        //default:
        //cout << getch() << endl;

        break;

        }

        }while(c<1);

    t1.join(); // Wait for t1 to join
    t2.join(); // Wait for t2 to join
    t3.join(); // Wait for t3 to join
    t4.join(); // Wait for t4 to join
    t5.join(); // Wait for t5 to join

return 0;
}
