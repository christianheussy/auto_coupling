#ifndef _detect_h
#define _detect_h

#include <iostream>
#include <cmath>
#include <sstream>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <iomanip>
#include "LeddarC.h"
#include "LeddarProperties.h"

#define ARRAY_LEN( a )  (sizeof(a)/sizeof(a[0]))
//#include <ctype.h>


void detect(float res[8][3], float dist[16]);

#endif
