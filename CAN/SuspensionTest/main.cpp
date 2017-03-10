#include <iostream>
#include "canlib.h"
#include <stdio.h>
#include <conio.h>
#include <bitset>
#include <unistd.h>
#include <thread>

    using namespace std;

    int CheckStat(canStatus stat);

    int command = 0;

    bool Write= true;

    canHandle hnd4;//Declare circuit handle
    canHandle hnd2;
    canStatus stat; //Declare status


  // Create ASC6 initial level command message
  // Range -600 to 600
  // Initial preset: 0
  long ASC6_ID = 0x18D14F27;
  unsigned char * ASC6_DATA = new unsigned char[8];
  unsigned int ASC6_DLC = 8;
  unsigned int ASC6_FLAG = canMSG_EXT;

  // ASC2 command message w/ nominal level request axle set to preset level
  long ASC2_ID = 0x18D24F27;
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


  void setHeight()
  {
    hnd4 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);
    stat=canBusOn(hnd4);
    CheckStat(stat);

    // read current from angle from ASC3
    stat=canReadSpecific(hnd4, ASC3_ID, ASC3_DATA, ASC3_DLC, ASC3_FLAG, ASC3_TIME);
    CheckStat(stat);

    //need to take in desired height
    //calculate command based on ASC3 reception

    while(Write){
    ASC6_DATA[4] = ASC3_DATA[4] + (0xFF & command);
    ASC6_DATA[5] = ASC3_DATA[5] + 0x0F;

    stat = canWrite(hnd4, ASC6_ID, ASC6_DATA, ASC6_DLC, ASC6_FLAG);
    cout << "yo" << endl;
    this_thread::yield();
    this_thread::sleep_for (chrono::milliseconds(100));
    }

  }


  int main()
  {

    canInitializeLibrary(); //Initialize driver

    hnd2 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);
    stat=canBusOn(hnd2);
    CheckStat(stat);

    int c = 0;

    ASC2_DATA[0] = 0x04; //message ASC2 set to preset level

    int value = 0;

    while(value<1)
    {

    stat=canReadSpecific(hnd2, ASC1_ID, ASC1_DATA, ASC1_DLC, ASC1_FLAG, ASC1_TIME);
    CheckStat(stat);
    if (stat == 0)
        value++;

    //if ((ASC1_DATA[0] & 0x0F) != ASC2_DATA[0]){


    stat = canWrite(hnd2, ASC2_ID, ASC2_DATA, ASC2_DLC, ASC2_FLAG);
    CheckStat(stat);

    }

    Write = true;

    std::thread t1 (setHeight);


     do{
        switch(getch()) { // the real value

        case 72:
           // cout << "arrow up" << endl;
            command = command + 100;
            break;

        case 80:
           // cout << "arrow down" << endl;
        command = command - 100;
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

    return 0;

  }
