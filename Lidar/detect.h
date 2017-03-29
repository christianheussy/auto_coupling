#ifndef _detect_h
#define _detect_h

//#include <ctype.h>

#define DETS 16
#define WID  2.6
#define NUM  DETS - 1
#define VIA  acos((double)-1)/4
#define FAC 1

void detect(double res[DETS/2][3], double dist[DETS]);

#endif
