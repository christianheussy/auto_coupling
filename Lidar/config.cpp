#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <sstream>
#include <math.h>

using namespace std;

int DETS;
float WID;
float VIA;
//Path Variables
float RES;
float RMIN;
float L;
//Lower LIDAR Variables
float RATE;
float INIT_ANG;
float BEAM;
float THRESH;
int SPEED;
//Debug
int DEBUG;
int SIMPLE;
float OFFSET;
float STEER;

void config()
{
    string name;
    string va;
    float value;
    //Detect Variables


    ifstream myfile;
    myfile.open("configer");

    while(getline(myfile,name,' '))
    {
    getline(myfile,va);
    istringstream iss(va);
    iss >> value;

    if(name == "DETS")
        DETS = value;
    else if(name == "WID")
        WID = value;
    else if(name == "VIA")
        VIA = value*acosf((float)-1)/(180.0);
    else if(name == "RES")
        RES = value;
    else if(name == "RMIN")
        RMIN = value;
    else if(name == "L")
        L = value;
    else if(name == "RATE")
        RATE = value;
    else if(name == "INIT_ANG")
        INIT_ANG = value*acosf((float)-1)/(180.0);
    else if(name == "BEAM")
        BEAM = value*acosf((float)-1)/(180.0);
    else if(name == "THRESH")
        THRESH = value;
    else if(name == "SPEED")
        SPEED = value;
    else if(name == "DEBUG")
        DEBUG = value;
    else if(name == "SIMPLE")
        SIMPLE = value;
    else if(name == "STEER")
        STEER = value;
    else if(name == "LID_ONLY")
        LID_ONLY = value;
    }

    OFFSET = ((float)SPEED /3600)*(1.0/RATE)*tanf(INIT_ANG);
}
