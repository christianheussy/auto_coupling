#pragma once
#include "canlib.h"

#include <atomic>
#include <thread>
#include <stdio.h>

// Variables for steering thread

// Command value (range depends on mode)
extern std::atomic<int> steering_command;
// Steering Mode
extern std::atomic<int> steering_mode;

// Variables for speed thread

// 0 is reverse, 1 is forwards
extern std::atomic<int> direction;
// 1 bit = .001 kph
extern std::atomic<int> speed_command;
// Activate when driver has foot on brake and shifts into gear
extern std::atomic<int> auto_park_enable;

// Variables for braking thread
// Flag to apply brakes and signal brakes are being externally applied
extern std::atomic<int> braking_active;   

// Variables for suspension thread
extern std::atomic<int> requested_height;
extern std::atomic<int> height_control_enable;

// Variables for reading off CAN bus
extern std::atomic<int> requested_gear;
extern std::atomic<int> brake_pedal;

extern std::atomic<int> system_enable;  

// Flag used to signal threads to quit execution
extern std::atomic<int> exit_flag;        

// CAN lib specific variables
extern canHandle hnd1, hnd2, hnd3, hnd4, hnd5;      // Declare CanLib Handles and Status
extern canStatus stat;

// Character arrays for CAN data
extern unsigned char * steer_data;
extern unsigned char * speed_data;
extern unsigned char * brake_data;
extern unsigned char * brake_pedal_data;
extern unsigned char * current_gear_data;

extern long brake_pedal_ID;
extern long current_gear_ID;

// Variables that are used by the CAN read function
extern unsigned int gear_DLC, brake_pedal_DLC;
extern unsigned int gear_FLAG, brake_pedal_FLAG;
extern unsigned long gear_TIME, brake_pedal_TIME;

int CheckStat(canStatus stat);
void Steering();
void Transmission();
void Brakes();
void Reader();
