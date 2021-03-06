// writing on a text file
#include <iostream>
#include <fstream>
#include <chrono>
#include "canlib.h"

using namespace std;

int CheckStat(canStatus stat);

int main () {

    hnd1 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);        // Open channel for Steer thread
    stat=canSetBusParams(hnd1, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
    CheckStat(stat);
    stat=canSetBusOutputControl(hnd1, canDRIVER_NORMAL);        //Set driver type normal
    CheckStat(stat);
    stat=canBusOn(hnd1);                                        //Take channel on bus
    CheckStat(stat);

    unsigned int VDC2_DLC, VDC2_FLAG;
    unsigned long VDC2_TIMESTAMP;
    unsigned char * VDC2_DATA = new unsigned char[8];

    ofstream myfile ("example.txt");
    if (myfile.is_open())
    while(true){
    stat = canReadSpecific(hnd, VDC2_ID, VDC2_DATA, &VDC2_DLC, &VDC2_FLAG, &VDC2_TIMESTAMP)
    lateral_acceleration = (VDC2_DATA[7] * 0.1) - 12.5
    myfile << count << endl;
    this_thread::sleep_for (chrono::milliseconds(10));
    }
    myfile.close();

    else cout << "Unable to open file";
  return 0;
}
