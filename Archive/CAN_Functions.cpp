//
//  CAN_Functions.cpp
//  
//
//  Created by Christian Heussy on 3/27/17.
//
//




// This thread is used to control the steering system
// Inputs steering mode, steering command value

void Steering() {
    
    static std::atomic<int> steering_command{0}; // Command value (range depends on mode)
    static std::atomic<int> steering_mode{1};    // Steering Mode (position, torque, etc..)
    
    unsigned char * steer_data = new unsigned char[8];
    
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
    
    while(exit_flag!=true)
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
        CheckStat(stat);
        
        this_thread::yield();
        this_thread::sleep_for (chrono::milliseconds(10));
        
    }
    
    stat = canBusOff(hnd1); // Take channel offline
    CheckStat(stat);
    canClose(hnd1);
}



void Transmission() {// Thread used to control the speed using the transmission
    
    unsigned char * speed_data = new unsigned char[3];
    
    static std::atomic<int> direction{0};        // 0 is reverse, 1 is forwards
    static std::atomic<int> speed_command{0};    // 1 bit = .001 kph
    static std::atomic<int> auto_park_enable{0}; // Activate when driver has foot on brake and shifts
    
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
        if (SpeedOn)
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
            CheckStat(stat);
        }
        this_thread::yield();
        this_thread::sleep_for (chrono::milliseconds(10));
    }
    stat = canBusOff(hnd2); // Take channel offline
    CheckStat(stat);
    canClose(hnd2);
}


void Brakes() {//Thread to Apply Brakes
    
    static std::atomic<int> braking_active{0};
    
    unsigned char * brake_data = new unsigned char[8];
    
    hnd5 = canOpenChannel(1, canOPEN_REQUIRE_EXTENDED);        // Open channel for speed control
    stat=canSetBusParams(hnd5, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
    CheckStat(stat);
    stat=canSetBusOutputControl(hnd5, canDRIVER_NORMAL);        // set driver type normal
    CheckStat(stat);
    stat=canBusOn(hnd5);                                        // take channel on bus and start reading messages
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
        //set bits 6.1 and 6.4 to 1
        
        
        if(braking_active == 1)
        {
            brake_data[5] = ((0xF & 0x9) );
        }
        else if (braking_active == 0){
            brake_data[5] = 0;
        }
        
        stat = canWrite(hnd5, Brake_ID, brake_data, Brake_DL, {});
        this_thread::yield();
        this_thread::sleep_for (chrono::milliseconds(10));
    }
    stat = canBusOff(hnd5); // Take channel offline
    CheckStat(stat);
    canClose(hnd5);
}

int set_steering_command(int mode, int command){
    test_lock.lock();
    steer_data[0] = mode;
    steer_data[1] =  (command & 0x000000FF);
    steer_data[2] = ((command & 0x0000FF00) >> 8);
    steer_data[3] = ((command & 0x00FF0000) >> 16);
    steer_data[4] = ((command & 0xFF000000) >> 24);
    steer_data[5] = 0xFF;
    steer_data[6] = 0xFF;
    test_lock.unlock();
}

