#ifndef _config_h
#define _config_h

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <fstream>

extern int DETS;
extern float WID;
extern float VIA;
//Path Variables
extern float RES;
extern float RMIN;
extern float L;
//Lower LIDAR Variables
extern float RATE;
extern float INIT_ANG;
extern float BEAM;
extern float THRESH;
extern int SPEED;
//Debug
extern int DEBUG;
extern int SIMPLE;
extern float OFFSET;
extern float STEER;

void config();



#endif
