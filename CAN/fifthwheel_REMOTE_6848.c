#include <iostream>
#include "canlib.h"
#include <stdio.h>
#include <conio.h>
#include <bitset>
#include <unistd.h>
#include <thread>

using namespace std;

int CheckStat(canStatus stat);
//in: none
//out: returns true
bool fifthwheel()
{
    canInitializeLibrary();
    canStatus stat;
    canHandle hnd;

    long Command_ID = 0x649424D;   //110001111000000010010   //Need SA
    unsigned int Command_DL = 8;            //Data length
    unsigned int Command_FLAG = canMSG_EXT; //Indicates extended ID
    unsigned char * messagedata = new unsigned char[8];
    unsigned long * time; //Indicates extended ID
    unsigned long timeout = 10;

    hnd = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);

    stat=canReadWait(hnd, Command_ID, messagedata, Command_DL, Command_FLAG, time, timeout);
    if (CheckStat < 0)
        {
        printf("Error");
        }
    printf( "Rx:%d %d %d %d %d %d %d %d \n" , messagedata[0],
               messagedata[1], messagedata[2], messagedata[3], messagedata[4],
               messagedata[5], messagedata[6], messagedata[7]);

    //PSUEDO
    //Translate message to determine if fifthwheel sensor is locked.
    if (coupled)
        return true;
        //store message
    else
        return false;
}
