#include "canlib.h"
#include <iostream>
#include <stdio.h>

int CheckStat(canStatus stat)
{
      char buf[100];
      if (stat != canOK) {
        buf[0] = '\0';
        canGetErrorText(stat, buf, sizeof(buf));
        printf("Failed, stat=%d (%s)\n", (int)stat, buf);
        //exit(1);
        return 0;
      }
}
