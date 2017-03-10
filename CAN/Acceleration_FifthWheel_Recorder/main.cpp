// writing on a text file
#include <iostream>
#include <fstream>
#include <chrono>
#include "canlib.h"
#include <thread>

using namespace std;

canHandle hnd1; // Declare CanLib Handles and Status
canStatus stat;

int CheckStat(canStatus stat);

long FWSS_ID = 0x18FDAE27;
unsigned int FWSS_DLC, FWSS_FLAG;
unsigned long FWSS_TIMESTAMP;
unsigned char * FWSS_DATA = new unsigned char[8];

long VDC2_ID = 0x18F00927;
unsigned int VDC2_DLC, VDC2_FLAG;
unsigned long VDC2_TIMESTAMP;
unsigned char * VDC2_DATA = new unsigned char[8];

float lateral_acceleration;
bool WriteOn;

int error, ready_indicator, status_indicator, slider_position, lock;

void Writer()
{
    
    hnd1 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);        // Open channel for Steer thread
    stat=canSetBusParams(hnd2, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
    CheckStat(stat);
    stat=canSetBusOutputControl(hnd2, canDRIVER_NORMAL);        //Set driver type normal
    CheckStat(stat);
    stat=canBusOn(hnd2);                                        //Take channel on bus
    CheckStat(stat);
    
    
    
    ofstream myfile ("Recording1.txt");  // opening log file
    if (myfile.is_open())
    {
        myfile << "Acceleration_(m/s),Error_Status,Ready_Indicator,Status_Indicator,Slider_Position,Lock,\n"; //wrting header

    
        while(WriteOn)
        {
            
            stat = canReadSpecific(hnd2, VDC2_ID, VDC2_DATA, &VDC2_DLC, &VDC2_FLAG, &VDC2_TIMESTAMP);
            lateral_acceleration = (VDC2_DATA[7] * 0.1) - 12.5;
        
            stat = canReadSpecific(hnd2, FWSS_ID, FWSS_DATA, &FWSS_DLC, &FWSS_FLAG, &FWSS_TIMESTAMP)
            
        error = (FWSS_DATA[0] & 0xF0);
        ready_indicator = (FWSS_DATA[0] & 0xC);
        status_indicator = (FWSS_DATA[0] & 0x3);
        slider_position = FWSS_DATA[1] * 10; // 10mm per bit
        lock = (FWSS_DATA[1] & 0xCO);
        
            myfile << lateral_acceleration << ",";
            myfile << error << ",";
            myfile << ready_indicator << ",";
            myfile << status_indicator << ",";
            myfile << slider_position << ",";
            myfile << lock << "," << endl;
        
    this_thread::yield();
    this_thread::sleep_for (chrono::milliseconds(100));
        }
    }
    else cout << "Unable to open file";

    
    myfile.close();
    
    stat = canBusOff(hnd2); // Take channel offline
    CheckStat(stat);
    canClose(hnd2);
}

int main () {
    canInitializeLibrary(); //Initialize driver
    hnd1 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);        // Open channel for Steer thread
    stat=canSetBusParams(hnd1, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
    CheckStat(stat);
    stat=canSetBusOutputControl(hnd1, canDRIVER_NORMAL);        //Set driver type normal
    CheckStat(stat);
    stat=canBusOn(hnd1);                                        //Take channel on bus
    CheckStat(stat);
    
    // Request FWSS
    // Check FWSS broadcasting
    // If not request again
    
    long request_id = 0x18EA  27; //need FWSS source address
    unsigned int request_DLC = 3;
    unsigned char * request_data = new unsigned char[3];
    request_data[0] = 0xAE;
    request_data[1] = 0xFD;
    request_data[2] = 0x00;
    
    unsigned long timeout = 1000;
    
    stat = -1;
    int count = 0;
    int c = 0;
    
    while( stat < 0 )
        {
    
        canWrite(hnd1, request_id, request_data, request_DLC);
    
        stat = canReadSyncSpecific(hnd1, FWSS_ID, timeout);
        
        count++;
        
        if (count == 5)
            {
            cout << "Unable to register fifth wheel smart sensor" << endl;
            break;
            }
        }
    
        if (count < 5)
        {
            cout << "Received FWSS signal, ready to begin recording" << endl;
        }
    
    system("PAUSE");
    
    std::thread t1 (Writer);
    
    do{
        switch(getch()) { // the real value
                
            case 72:
                // cout << "arrow up" << endl;
                break;
                
            case 27: //Exit Key
                c++;
                WriteOn = false;
                
                break;
                
            case 13: // enter
                break;
                
        }
        
    }while(c<1);
    
    t1.join();

    stat = canBusOff(hnd1); // Take channel offline
    CheckStat(stat);
    canClose(hnd1);
    
    

  return 0;
}
