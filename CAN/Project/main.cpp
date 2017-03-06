#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>
#include "canlib.h"
#include <stdio.h>
#include <conio.h>

using namespace std;

mutex test_lock;

// This program is used to test the individual CAN control functions
// Steering
// Suspension
// Brakes
// Transmission

int CheckStat(canStatus stat); // Forward Declaration for check stat function

bool SteerOn; //Bool values for threads
bool SpeedOn;
bool ReadOn;
bool BrakeOn;

int mode;

int command = 0;

unsigned char * steer_data = new unsigned char[8];
unsigned char * Drive_DATA = new unsigned char[3];
unsigned char * brake_data = new unsigned char[8];
unsigned char * VDC2_DATA = new unsigned char[8];

canHandle hnd1, hnd2, hnd3, hnd4, hnd5; // Declare CanLib Handles and Status
canStatus stat;

unsigned char data[8];

void Send_Steer() { // This thread sends torque commands to the steering
    int message_count{}, checksum_temp{}, checksum_calc{};

    long steering_command_ID = 0x18FFEF27;
    unsigned int steering_command_DLC = 8;            //Data length
    unsigned int steering_command_FLAG = canMSG_EXT; //Indicates extended ID

    hnd1 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);        // Open channel for Steer thread
    stat=canSetBusParams(hnd1, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
    CheckStat(stat);
    stat=canSetBusOutputControl(hnd1, canDRIVER_NORMAL);        //Set driver type normal
    CheckStat(stat);
    stat=canBusOn(hnd1);                                        //Take channel on bus
    CheckStat(stat);

    while (true)
    {
        if (SteerOn)
        {
            message_count++;
            if (message_count > 15)
            {
            message_count = 0;
            }

        //Check Sum Calculation
        checksum_temp = steer_data[0] + steer_data[1] + steer_data[2] +
        steer_data[3] + steer_data[4] + steer_data[5] + steer_data[6] +
        (Command_ID  & 0x000000FF) +
        ((Command_ID & 0x0000FF00) >> 8)  +
        ((Command_ID & 0x00FF0000) >> 16) +
        ((Command_ID & 0xFF000000) >> 24) +
        (message_count);

        checksum_calc = ((checksum_temp >> 4) + checksum_temp) & 0x000F;

        steer_data[7] =  (checksum_calc << 4) + (message_count); // put checksum into last byte

        stat=canWriteWait(hnd1, steering_command_ID, steer_data, steering_command_DLC, steering_command_FLAG,50);
        CheckStat(stat);

//        printf( "Tx:%d %d %d %d %d %d %d %d\n" , steer_data[0],
//               steer_data[1], steer_data[2], steer_data[3], steer_data[4],
//               steer_data[5], steer_data[6], steer_data[7]);
        }
        this_thread::yield();
        this_thread::sleep_for (chrono::milliseconds(10));

    }
    stat = canBusOff(hnd1); // Take channel offline
    CheckStat(stat);
    canClose(hnd1);
}
    

//void Send_Speed()  // Thread used to control the speed using the transmission
//    {
//    long Drive_ID = 0x4FF5527; //0x4FF5527
//    unsigned int Drive_DL = 3; //3 Bytes ??
//    unsigned int Drive_FLAG = canMSG_EXT; //Indicates extended ID
//
//    hnd2 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);        // Open channel for speed control
//    stat=canSetBusParams(hnd2, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
//        CheckStat(stat);
//    stat=canSetBusOutputControl(hnd2, canDRIVER_NORMAL);        // set driver type normal
//        CheckStat(stat);
//    stat=canBusOn(hnd2);                                        // take channel on bus and start reading messages
//        CheckStat(stat);
//
//    while (SpeedOn)
//        {
//     Using PGN FF55
//     Repetition Rate = 10ms
//     Priority 6 (110) hopefully?
//     Binary ID: 001 0 0 1111111101010101 00100111
//     Data
//    01 // Enable = 1 (2  bits)
//    0000110110101100 // Speed 0-3.5kph (16 bits)
//    00 // 0 = reverse, 1 = forward (2 bits)
//    00 // Brakes used by automated system (yes / no)
//    00 // always 00 (2 bits)
//
//            stat=canWrite(hnd2, Drive_ID, Drive_DATA, Drive_DL, Drive_FLAG);
//            cout << "Data = " << Drive_DATA << endl;
//            CheckStat(stat);
//            this_thread::yield();
//            this_thread::sleep_for (chrono::milliseconds(10));
//        }
//
//    stat = canBusOff(hnd2); // Take channel offline
//    CheckStat(stat);
//    canClose(hnd2);
//    }

void Request_FFF7()  // Thread used to control the speed using the transmission
    {
    long reAX_diagnostic_id = 0x18EA1327; //0x4FF5527
    unsigned int reAX_diagnostic_DLC = 3; //3 Bytes ??
    
    hnd2 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);        // Open channel for speed control
    stat=canSetBusParams(hnd2, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
        CheckStat(stat);
    stat=canSetBusOutputControl(hnd2, canDRIVER_NORMAL);        // set driver type normal
        CheckStat(stat);
    stat=canBusOn(hnd2);                                        // take channel on bus and start reading messages
        CheckStat(stat);

        unsigned char * request_data = new unsigned char[3];
        request_data[0] = 0xF7;
        request_data[1] = 0xFF;
        request_data[2] = 0x00;

    while (true)
    {
        if(FFF7_Request)
        {
            stat=canWrite(reAX_diagnostic_id, request_data, reAX_diagnostic_DLC, );
            //cout << "Data = " << Drive_DATA << endl;
            CheckStat(stat);
        }
    this_thread::yield();
    this_thread::sleep_for (chrono::milliseconds(1000));
    }

        stat = canBusOff(hnd2); // Take channel offline
        CheckStat(stat);
        canClose(hnd2);
    }

//void Read_Any() // Thread to test reading singals off fake bus
//    {
//    unsigned long timeout{200};
//
//    hnd3 = canOpenChannel(0, 0);        // Open channel for Steer thread
//    stat=canSetBusParams(hnd1, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
//        CheckStat(stat);
//
//    stat=canSetBusOutputControl(hnd3, canDRIVER_NORMAL);        //Set driver type normal
//        CheckStat(stat);
//
//    stat=canBusOn(hnd3);                                        //Take channel on bus
//        CheckStat(stat);
//
//        long responseA = 0x18FFF113;
//
//
//
//        unsigned int dlc, flags;
//        unsigned long timestamp;
//
//    while (ReadOn)
//        {
//
//        stat = canReadSpecificSkip(hnd3, Response_Msg_B, &data, &dlc, &flags, &timestamp);
//        mode = (data[0] & 0xf);
//        CheckStat(stat);
//
//        printf( "Rx:%d %d %d %d %d %d %d %d\n" , data[0],
//       data[1], data[2], data[3], data[4],
//       data[5], data[6], data[7]);
//
//
//        this_thread::yield();
//        this_thread::sleep_for (chrono::milliseconds(10));
//        }
//    }

void Apply_Brake() //Thread to Apply Brakes
    {
    int brake_pressure_value = 20; // 4 bar
    int brake_pressure_command;

    brake_pressure_command = (brake_pressure_value & 0x000000FF);
    long Brake_ID = 0x750;
    unsigned int Brake_DL = 8; //3 Bytes ??
    unsigned int Brake_FLAG = canMSG_EXT; //Indicates extended ID

    brake_data[0] = brake_pressure_command; //Front Left
    brake_data[1] = brake_pressure_command; //Front RIght
    brake_data[2] = brake_pressure_command; //Rear Left
    brake_data[3] = brake_pressure_command; //Rear Right
    brake_data[4] = brake_pressure_command;
    brake_data[5] = (0xF & 0x9); //set bits 6.1 and 6.4 to 1
    brake_data[6] = 0;
    brake_data[7] = 0;

    while (true)
    {
      if(BrakeOn)
      {stat = canWrite(hnd5, Brake_ID, Brake_DL, Brake_FLAG, brake_data);
      }
        this_thread::yield();
        this_thread::sleep_for (chrono::milliseconds(10));
    }
        stat = canBusOff(hnd5); // Take channel offline
        CheckStat(stat);
        canClose(hnd5);
    }



void read_VDC2()
    {
    unsigned int VDC2_DLC, VDC2_FLAG;
    unsigned long VDC2_TIMESTAMP;
    stat = canReadSpecific(hnd, VDC2_ID, VDC2_DATA, &VDC2_DLC, &VDC2_FLAG, &VDC2_TIMESTAMP)
    
    lateral_acceleration = (VDC2_DATA[7] * 0.1) - 12.5
    }


int set_steering_command(int mode, int command)
    {
        test_lock.lock();
        steer_data[0] = mode;
        steer_data[1] =  (command & 0x000000FF);
        steer_data[2] = ((command & 0x0000FF00) >> 8);
        steer_data[3] = ((command & 0x00FF0000) >> 16);
        steer_data[4] = ((command & 0xFF000000) >> 24);
        steer_data[5] = 0xFF;
        steer_data[6] = 0xFF;
        test_lock.unlock();
    }

int main() {

    canInitializeLibrary(); //Initialize driver

    set_steering_command(1,12000);

//	Drive_DATA[0] = 01000011;
//	Drive_DATA[1] = 01101011;
//	Drive_DATA[2] = 00000000;

	// int Drive_DATA = 01000011 01101011 00000000;


    SteerOn = false;
    SpeedOn = false;
    BrakeOn  = false;

    std::thread t1 (Send_Steer); // Start thread for steering control
    std::thread t2 (Send_Speed); // Start thread for transmission control
    std::thread t3 (Apply_Brake);  // Start thread to read

    int c=0;
    int gain = 10;
    cout << endl;
    cout << "Use the up and down arrow keys to adjust the gain" << endl;
    cout << "Use the left and right arrow keys to change the steering wheel position." << endl;

    do{
        switch(getch()) { // the real value

        case 72:
           // cout << "arrow up" << endl;

            cout << "Gain =" << gain << endl;

            break;
        case 80:
           // cout << "arrow down" << endl;
            gain = gain - 10;
            cout << "Gain =" << gain << endl;
            break;

        case 77: //Right Arrow
            command = command + 1000;
            set_steering_command(1,command);
                cout << "Command + 1000" << endl;
            break;

        case 75: //Left Arrow
            command = command - 1000;
            set_steering_command(1,command);
                cout << "Command - 1000" << endl;
            break;

        case 27: //Exit Key
            c++;
            SteerOn = false;
            ReadOn = false;
            break;

        case 13: // enter
            break;

        }

        }while(c<1);

    t1.join();    // Wait for t1 to finish
    t2.join(); // Wait for t2 to finish
    t3.join(); // Wait for t3 to join


return 0;

}
