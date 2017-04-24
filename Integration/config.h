#ifndef _config_h
#define _config_h

#include <string.h>

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
extern float L_x_U;
extern float L_y_F;
//Debug
extern int DEBUG;
extern int SIMPLE;
extern float OFFSET;
extern float STEER;
extern int LID_ONLY;
extern std::string UPPER;
extern std::string LOWER;

void config();



#endif
