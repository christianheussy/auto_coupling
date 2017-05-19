#include <iostream>
#include "canlib.h"
#include <stdio.h>
#include <conio.h>
#include <bitset>
#include <unistd.h>
#include <thread>

using namespace std;

int CheckStat(canStatus stat);

int command = 32000;

bool Write= true;

canHandle hnd4, hnd2, hnd3;//Declare circuit handle
canStatus stat;


// Create ASC6 initial level command message
// Range -600 to 600
// Initial preset: 0


// ASC2 command message w/ nominal level request axle set to preset level
long ASC2_ID = 0xCD22f2b;
unsigned char * ASC2_DATA = new unsigned char[8];
unsigned int ASC2_DLC = 8; //Data length
unsigned int ASC2_FLAG = canMSG_EXT; //Indicates extended ID

long ASC6_ID = 0x18D12F27;
unsigned char * ASC6_DATA = new unsigned char[8];
unsigned int ASC6_DLC = 8;
unsigned int ASC6_FLAG = canMSG_EXT;

// Create ASC1 status message
long ASC1_ID = 0x18FE5A27;
unsigned char * ASC1_DATA = new unsigned char[8];
unsigned int * ASC1_DLC; //Data length
unsigned int * ASC1_FLAG;
unsigned long * ASC1_TIME;

// ASC3 status message
long ASC3_ID = 0x18FE5927;
unsigned char * ASC3_DATA = new unsigned char[8];
unsigned int * ASC3_DLC; //Data length
unsigned int * ASC3_FLAG; //Indicates extended ID
unsigned long * ASC3_TIME; // Timeout for read wait


void setHeight()
{
    hnd4 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);
    stat=canSetBusParams(hnd4, canBITRATE_250K, 0, 0, 0, 0, 0);
    stat=canSetBusOutputControl(hnd4, canDRIVER_NORMAL);
    stat=canBusOn(hnd4);
    CheckStat(stat);
    
    while(write == true)
    {
        ASC6_DATA[4] =  (command & 0x000000FF);
        ASC6_DATA[5] = ((command & 0x0000FF00) >> 8);
        
        stat = canWrite(hnd4, ASC6_ID, ASC6_DATA, ASC6_DLC, ASC6_FLAG);
        this_thread::yield();
        this_thread::sleep_for (chrono::milliseconds(100));
    }

    stat = canBusOff(hnd4); // Take channel offline
    CheckStat(stat);
    canClose(hnd4);
}


void Request()
{
    hnd3 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);
    stat=canSetBusParams(hnd3, canBITRATE_250K, 0, 0, 0, 0, 0);
    stat=canSetBusOutputControl(hnd3, canDRIVER_NORMAL);
    stat=canBusOn(hnd3);
    CheckStat(stat);
    stat=canBusOn(hnd3);
    CheckStat(stat);
    
    this_thread::yield();
    this_thread::sleep_for (chrono::milliseconds(100));
    
    ASC2_DATA[0] = 0;
    ASC2_DATA[1] = (1 << 4); //message ASC2 set to preset level
    
    stat = canWrite(hnd3, ASC2_ID, ASC2_DATA, ASC2_DLC, ASC2_FLAG);
}

int c = 0;



int main()
{
    
    canInitializeLibrary(); //Initialize driver
    hnd2 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);
    stat=canSetBusParams(hnd2, canBITRATE_250K, 0, 0, 0, 0, 0);
    stat=canSetBusOutputControl(hnd2, canDRIVER_NORMAL);
    stat=canBusOn(hnd2);
    CheckStat(stat);
    stat=canBusOn(hnd2);
    CheckStat(stat);
    
    
    int value = 0;
    
    //if(value< 0)
    //{
    
    std::thread t2 (Request);
    
    
    
    stat=canReadSpecific(hnd2, ASC1_ID, ASC1_DATA, ASC1_DLC, ASC1_FLAG, ASC1_TIME);
    CheckStat(stat);
    
    
    
    
    //if (stat == 0)
    //  value++;
    //}
    //if ((ASC1_DATA[0] & 0x0F) != ASC2_DATA[0]){
    
    
    //    stat = canWrite(hnd2, ASC2_ID, ASC2_DATA, ASC2_DLC, ASC2_FLAG);
    //  CheckStat(stat);
    
    //  }
    
    Write = true;
    
    std::thread t1 (setHeight);
    
    
    do{
        switch(getch()) { // the real value
                
            case 72:
                // cout << "arrow up" << endl;
                command = command + 100;
                cout << "up" << endl;
                break;
                
            case 80:
                // cout << "arrow down" << endl;
                command = command - 100;
                cout << "down" << endl;
                break;
                
            case 27: //Exit Key
                c++;
                Write = false;
                break;
                
            case 13: // enter
                break;
        }
    }while(c<1);
    
    t1.join();
    t2.join();
    
    stat = canBusOff(hnd2); // Take channel offline
    CheckStat(stat);
    canClose(hnd2);
    
    return 0;
    
}
