#include "can.h"

// Variables for steering thread
//
// Command value (range depends on mode)
std::atomic<int> steering_command{0};
// Steering Mode
std::atomic<int> steering_mode{2};

// Variables for speed thread
//
// 0 is reverse, 1 is forwards
std::atomic<int> direction{0};
// 1 bit = .001 kph
std::atomic<int> speed_command{0};
// Activate when driver has foot on brake and shifts into gear
std::atomic<int> auto_park_enable{0};

// Variables for braking thread
//
// Flag to apply brakes and signal brakes are being externally applied
std::atomic<int> braking_active{0};   

// Variables for suspension thread
//
std::atomic<int> requested_height{0};
std::atomic<int> height_control_enable{0};

// Variables for reading off CAN bus
//
std::atomic<int> requested_gear{0};
std::atomic<int> brake_pedal{0};

std::atomic<int> system_enable{0};  
// Flag used to signal threads to quit execution
std::atomic<int> exit_flag{0};        

// CAN lib specific variables
//
// Declare CanLib Handles and Status
canHandle hnd1, hnd2, hnd3, hnd4, hnd5;
canStatus stat;

// Character arrays for CAN data
unsigned char * steer_data = new unsigned char[8];
unsigned char * speed_data = new unsigned char[3];
unsigned char * brake_data = new unsigned char[8];
unsigned char * brake_pedal_data = new unsigned char[8];
unsigned char * current_gear_data = new unsigned char[8];

long brake_pedal_ID  = 0x18F0010B; // Id for brake signal
long current_gear_ID = 0x18F00503; // Id for transmission gear signal

// Variables that are used by the CAN read function
unsigned int gear_DLC, brake_pedal_DLC;
unsigned int gear_FLAG, brake_pedal_FLAG;
unsigned long gear_TIME, brake_pedal_TIME;

int CheckStat(canStatus stat){
    char buf[100];
    if (stat != canOK)
    {
        buf[0] = '\0';
        canGetErrorText(stat, buf, sizeof(buf));
        printf("Failed, stat=%d (%s)\n", (int)stat, buf);
        return 0;
    }
}

void Steering()
{
    int message_count{}, checksum_temp{}, checksum_calc{};

    long steering_command_ID = 0x18FFEF27;
    unsigned int steering_command_DLC = 8;            //Data length
    unsigned int steering_command_FLAG = canMSG_EXT;  //Indicates extended ID

    hnd1 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);        // Open channel for Steer thread
    stat=canSetBusParams(hnd1, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
    CheckStat(stat);
    stat=canSetBusOutputControl(hnd1, canDRIVER_NORMAL);        //Set driver type normal
    CheckStat(stat);
    stat=canBusOn(hnd1);                                        //Take channel on bus
    CheckStat(stat);

    while(true)
    {
        message_count++;
        if (message_count > 15)
        {
            message_count = 0;
        }
        steer_data[0] = steering_mode;
        steer_data[1] =  (steering_command & 0x000000FF);
        steer_data[2] = ((steering_command & 0x0000FF00) >> 8);
        steer_data[3] = ((steering_command & 0x00FF0000) >> 16);
        steer_data[4] = ((steering_command & 0xFF000000) >> 24);
        steer_data[5] = 0xFF;
        steer_data[6] = 0xFF;

        //Check Sum Calculation
        checksum_temp = steer_data[0] + steer_data[1] + steer_data[2] +
        steer_data[3] + steer_data[4] + steer_data[5] + steer_data[6] +
        (steering_command_ID  & 0x000000FF) +
        ((steering_command_ID & 0x0000FF00) >> 8)  +
        ((steering_command_ID & 0x00FF0000) >> 16) +
        ((steering_command_ID & 0xFF000000) >> 24) +
        (message_count);

        checksum_calc = ((checksum_temp >> 4) + checksum_temp) & 0x000F;

        steer_data[7] =  (checksum_calc << 4) + (message_count); // put checksum into last byte

        stat=canWriteWait(hnd1, steering_command_ID, steer_data, steering_command_DLC, steering_command_FLAG,50);
        //CheckStat(stat);

        if (exit_flag == 1){
            break;
        }

		std::this_thread::yield();
		std::this_thread::sleep_for (std::chrono::milliseconds(10));

    }

    stat = canBusOff(hnd1); // Take channel offline
    CheckStat(stat);
    canClose(hnd1);
}


void Transmission() {// Thread used to control the speed using the transmission

    long Drive_ID = 0x18FF552B;
    unsigned int Drive_DL = 8;
    unsigned int Drive_FLAG = canMSG_EXT;
    hnd2 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);        // Open channel for speed control
    stat=canSetBusParams(hnd2, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
    CheckStat(stat);
    stat=canSetBusOutputControl(hnd2, canDRIVER_NORMAL);        // set driver type normal
    CheckStat(stat);
    stat=canBusOn(hnd2);                                        // take channel on bus and start reading messages
    CheckStat(stat);
    while (true)
    {

        speed_data[0] = ((speed_command & 0x3F) << 2) + auto_park_enable;
        speed_data[1] = ((speed_command & 0x3FC0) >> 6);
        speed_data[2] = (braking_active << 4) + (direction << 2) + ((speed_command & 0xC000) >> 14);
        speed_data[3] = 0xFF;
        speed_data[4] = 0xFF;
        speed_data[5] = 0xFF;
        speed_data[6] = 0xFF;
        speed_data[7] = 0xFF;
        stat=canWrite(hnd2, Drive_ID, speed_data, Drive_DL, Drive_FLAG);
        //CheckStat(stat);

        if (exit_flag == 1){
            break;
        }

		std::this_thread::yield();
		std::this_thread::sleep_for (std::chrono::milliseconds(10));

    }
    stat = canBusOff(hnd2); // Take channel offline
    CheckStat(stat);
    canClose(hnd2);
}

void Brakes() {//Thread to Apply Brakes

    hnd3 = canOpenChannel(1, canOPEN_REQUIRE_EXTENDED);         // Open channel for speed control
    stat=canSetBusParams(hnd3, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
    CheckStat(stat);
    stat=canSetBusOutputControl(hnd3, canDRIVER_NORMAL);        // set driver type normal
    CheckStat(stat);
    stat=canBusOn(hnd3);                                        // take channel on bus and start reading messages
    CheckStat(stat);

    int brake_pressure_value = 15; // 8 bar
    int brake_pressure_command;

    brake_pressure_command = (brake_pressure_value & 0x000000FF);
    long Brake_ID = 0x750;
    unsigned int Brake_DL = 8; //3 Bytes ??
    unsigned int Brake_FLAG = {}; //Indicates extended ID

    while (true)
    {
        brake_data[0] = brake_pressure_command; //Front Left
        brake_data[1] = brake_pressure_command; //Front Right
        brake_data[2] = brake_pressure_command; //Rear Left
        brake_data[3] = brake_pressure_command; //Rear Right
        brake_data[4] = 0;

        if(braking_active == 1)
        {
            brake_data[5] = ((0xF & 0x9) );
        }
        else if (braking_active == 0){
            brake_data[5] = 0;
        }

        stat = canWrite(hnd3, Brake_ID, brake_data, Brake_DL, {});
		std::this_thread::yield();
		std::this_thread::sleep_for (std::chrono::milliseconds(10));

        if (exit_flag == 1){
            break;
            }

    }
    stat = canBusOff(hnd3); // Take channel offline
    CheckStat(stat);
    canClose(hnd3);
}

void Reader(){

    hnd5 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);        // Open channel for reading brake and current trans gear
    stat=canSetBusParams(hnd5, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
    CheckStat(stat);                                            // Check set bus parameters was success
    stat=canSetBusOutputControl(hnd5, canDRIVER_NORMAL);        // set driver type normal
    CheckStat(stat);                                            // Check driver initialized correctly
    stat=canBusOn(hnd5);                                        // take channel on bus and start reading messages
    CheckStat(stat);

    while(true)
    {
    // Read signal containing brake pedal status
    canReadSpecific(hnd5, brake_pedal_ID, brake_pedal_data, &brake_pedal_DLC, &brake_pedal_FLAG, &brake_pedal_TIME);

    // Read signal for transmission requested gear
    canReadSpecific(hnd5, current_gear_ID, current_gear_data, &gear_DLC, &gear_FLAG, &gear_TIME);

    requested_gear = current_gear_data[5];      // Retrieve ASCII character from data 6th byte

    brake_pedal = ((0xC0 & brake_pedal_data[0]) >> 6); // Retrieve two bit brake pedal status from from message

	std::this_thread::yield();
	std::this_thread::sleep_for (std::chrono::milliseconds(100));
    
		if(system_enable == 1 && requested_gear == 68 && brake_pedal == 1)
		{
			auto_park_enable = 1;
		}

        if (exit_flag == 1)
        {break;
        }
    }
    stat = canBusOff(hnd5); // Take channel offline
    CheckStat(stat);
    canClose(hnd5);
}
