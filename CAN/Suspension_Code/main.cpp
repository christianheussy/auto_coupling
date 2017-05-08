canHandle hnd5, hnd6;

void setHeight()
{
    hnd5 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);
    stat=canSetBusParams(hnd5, canBITRATE_250K, 0, 0, 0, 0, 0);
    stat=canSetBusOutputControl(hnd5, canDRIVER_NORMAL);
    stat=canBusOn(hnd5);
    CheckStat(stat);
    
    long ASC6_ID = 0x18D12F27;
    unsigned char * ASC6_DATA = new unsigned char[8];
    unsigned int ASC6_DLC = 8;
    unsigned int ASC6_FLAG = canMSG_EXT;
    
    while(true)
    {
        ASC6_DATA[4] =  (steering_command & 0x000000FF);
        ASC6_DATA[5] = ((steering_command & 0x0000FF00) >> 8);
        
        stat = canWrite(hnd4, ASC6_ID, ASC6_DATA, ASC6_DLC, ASC6_FLAG);
        this_thread::yield();
        this_thread::sleep_for (chrono::milliseconds(100));
        
        if (exit_flag == 1)
            break;
    }
    
    stat = canBusOff(hnd5); // Take channel offline
    CheckStat(stat);
    canClose(hnd5);
}


void Request()
{
    
    hnd3 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);
    stat=canSetBusParams(hnd3, canBITRATE_250K, 0, 0, 0, 0, 0);
    stat=canSetBusOutputControl(hnd3, canDRIVER_NORMAL);
    stat=canBusOn(hnd3);
    CheckStat(stat);
    stat=canBusOn(hnd3);
    CheckStat(stat);
    
    this_thread::yield();
    this_thread::sleep_for (chrono::milliseconds(100));
    
    ASC2_DATA[0] = 0;
    ASC2_DATA[1] = (1 << 4); //message ASC2 set to preset level
    
    stat = canWrite(hnd3, ASC2_ID, ASC2_DATA, ASC2_DLC, ASC2_FLAG);
    
    
    
}
