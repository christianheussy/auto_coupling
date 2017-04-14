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
static std::atomic<int> steering_mode{1};    // Steering Mode
static std::atomic<int> steering_desired{0};    // Steering Mode

// Variables for speed thread
static std::atomic<int> direction{0};        // 0 is reverse, 1 is forwards
static std::atomic<int> speed_command{0};    // 1 bit = .001 kph
static std::atomic<int> auto_park_enable{0}; // Activate when driver has foot on brake and shifts into gear
static std::atomic<int> braking_active{0};   // Flag to tell transmission brakes are being externally applied

static std::atomic<int> exit_flag{0};

// Character arrays for can message data
unsigned char * steer_data = new unsigned char[8];
unsigned char * speed_data = new unsigned char[3];
unsigned char * brake_data = new unsigned char[8];

canHandle hnd1, hnd2, hnd3, hnd4, hnd5; // Declare CanLib Handles and Status
canStatus stat;

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

        if (exit_flag == 1){
            break;
            }

    }
    stat = canBusOff(hnd3); // Take channel offline
    CheckStat(stat);
    canClose(hnd3);
}

/*
void Read() {//Thread to Apply Brakes

    hnd4 = canOpenChannel(0, canOPEN_REQUIRE_EXTENDED);
    stat=canSetBusParams(hnd3, canBITRATE_250K, 0, 0, 0, 0, 0);
    CheckStat(stat);
    stat=canSetBusOutputControl(hnd4, canDRIVER_NORMAL);
    CheckStat(stat);
    stat=canBusOn(hnd4);
    CheckStat(stat);

    long Current_Gear_ID = 0x18F00527;
    unsigned int Gear_DLC; //3 Bytes ??
    unsigned int Gear_FLAG;
    unsigned long * Gear_TIME;
    unsigned char * Current_Gear_Data = new unsigned char[8];


    while (true)
    {


        stat = canReadSpecific(hnd4, Current_Gear_ID, Current_Gear_Data, &Gear_DLC, &Gear_FLAG,  &Current_Gear_Data)
        CheckStat(stat);

        current_gear = Current_Gear_Data[4];





        this_thread::yield();
        this_thread::sleep_for (chrono::milliseconds(100));

        if (exit_flag == 1){
            break;
        }

    }
    stat = canBusOff(hnd4); // Take channel offline
    CheckStat(stat);
    canClose(hnd4);
}
*/



void smoother()
{
    int increment = 100;
    int limit = 0;
    int counter = 0;

    while(true){

        limit = ((steering_desired - steering_command) / increment);




        if (limit > 0){
            for (int i = 0; i < limit; ++i)
            {
                //steering_command = steering_command + increment;
                //cout << "steering command=" << steering_command << endl;
                this_thread::yield();
                this_thread::sleep_for (chrono::milliseconds(500/limit));
            }
        }
        if (limit < 0){
            for (int i = 0; i > limit; --i)
            {
                //steering_command = steering_command - increment;
                //cout << "steering command=" << steering_command << endl;
                this_thread::yield();
                this_thread::sleep_for (chrono::milliseconds(500/abs(limit)));
            }

        }


             if (exit_flag == 1){
            break;
            }

    }

}



int main() {

    canInitializeLibrary(); //Initialize driver


    std::thread t1 (Steering); // Start thread for steering control
    std::thread t2 (Transmission); // Start thread for transmission control
    std::thread t3 (Brakes);  // Start thread to read
    std::thread t4 (smoother);


    int c=0;
    do{
        switch(getch()) { // the real value

        case 72: //Arrow Up
            auto_park_enable = 1;
            speed_command = speed_command + 500;
            cout << "Speed =" << speed_command*.001 << "kph" << endl;
            break;

        case 80: //Arrow Down
            speed_command = speed_command - 500;
            cout << "Speed =" << speed_command*.001 << "kph" << endl;
            break;

        case 77: //Right Arrow`

            steering_command = steering_command + 100;
            cout <<  "Steering Desired= " << steering_command << endl;
            break;

        case 75: //Left Arrow

            steering_command = steering_command - 100;
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

        case 27: //Exit Key
            c++;
            exit_flag = true;

            break;



       // default: cout << getch() << endl;
       // break;

        }

        }while(c<1);

    t1.join(); // Wait for t1 to finish
    t2.join(); // Wait for t2 to finish
    t3.join(); // Wait for t3 to join
    t4.join();


return 0;

}
