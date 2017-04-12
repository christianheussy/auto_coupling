#ifndef THREADS_H
#define THREADS_H

#include <thread>
#include <iostream>
#include <chrono>
#include "canlib.h"
#include <stdio.h>
#include <atomic>

void Steering();        // Forward declaration for steering

void Transmission();    // Forward declaration for speed control

void Brakes();          // Forward declaration for Brakes

int CheckStat(canStatus stat);

#endif
