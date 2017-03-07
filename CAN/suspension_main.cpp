#include <iostream>
#include "canlib.h"
#include <stdio.h>
#include <conio.h>
#include <bitset>
#include <unistd.h>
#include <thread>


    using namespace std;

    int CheckStat(canStatus stat);

    canHandle hnd4;//Declare circuit handle
    canHandle hnd2;
    canStatus stat; //Declare status

    bool SpeedOn;

   // unsigned char * Drive_DATA = new unsigned char[3];

  // Create ASC1 status message
  long * suspension_status1_id;  // What is ASC1 UD
  unsigned char * suspension_status1_data = new unsigned char[8];
  unsigned int * suspension_status1_dlc; //Data length
  unsigned int * suspension_status1_flag;
  unsigned long * suspension_status1_time; //Indicates extended ID
  unsigned long suspension_status1_timeout = 50; // Timeout for read wait

  // Create ASC6 initial level command message
  // Range -600 to 600
  // Initial preset: 0
  long suspension_command1_id = 0x1868AF27;  //0110011010001010111100100111;
  unsigned char * suspension_command1_data = new unsigned char[8];
  unsigned int suspension_command1_dlc = 8;
  unsigned int suspension_command1_flag = canMSG_EXT;

  // ASC2 command message w/ nominal level request axle set to preset level
  long suspension_command2_id = 0x18692F27; // 0110011010010010111100100111;
  unsigned char * suspension_command2_data = new unsigned char[8];
  unsigned int suspension_command2_dlc = 8; //Data length
  unsigned int suspension_command2_flag = canMSG_EXT; //Indicates extended ID
  unsigned long suspension_command2_timeout = 1000; // Timeout for read wait

  // ASC3 status message
  long ASC3id = 0x18FE5927; //11000111111100101100100100111;
  unsigned char * suspension_status2_id = new unsigned char[8];
  unsigned int suspension_status2_dlc = 8; //Data length
  unsigned int suspension_status2_flag = canMSG_EXT; //Indicates extended ID
  unsigned long suspension_status2_timeout = 1000; // Timeout for read wait

  void setPresetLevel(){

  hnd4 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);
  stat=canBusOn(hnd4);
  CheckStat(stat);
  unsigned long timestamp;
  suspension_command1_data[1] = suspension_command_data[1] & 0x04; //message ASC2 set to preset level

  stat=canReadWait(hnd4, suspension_status1_id, suspension_status1_data, suspension_status1_dlc, suspension_status1_flag, suspension_status1_time,suspension_status1_timeout);
    CheckStat(stat);
    
  while (suspension_status1_data[0] != suspension_command1_data[1]){
    stat = canWrite(hnd4,suspension_command1_id,suspension_command1_data, suspension_command1_dlc, suspension_command1_flag);
    if (CheckStat < 0)
        cout << "error" << endl;
  }
  }

  void setHeight(unsigned char &height)
  {
    hnd4 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);
    stat=canBusOn(hnd4);
    CheckStat(stat);

    unsigned char * suspensionHeight = new unsigned char[2];
    stat=canReadWait(hnd4, suspension_status2_id, suspension_status2_data, suspension_status2_dlc, suspension_status2_flag, suspension_status2_time,suspension_status2_timeout);
    if (CheckStat < 0)
        break;
    suspensionHeight[0] = suspension_status2_data[4] & 0xFF;
    suspensionHeight[1] = suspension_status2_data[5] & 0xFF;

    //for(int i = 4, i < 6, i++)

    suspension_command2_data[5] = height[0] & 0xFF;
    suspension_command2_data[6] = height[1] & 0xFF;

    //convert int height into char
    stat = canWrite(hnd4,suspension_command2_id, suspension_command2_data, suspension_command2_dlc, suspension_command2_flag);
    if (CheckStat(stat) < 0)
        break;
    else
        cout << "successful write" << endl;
    //decode message thats binary to a readable angle
    //set angle with ASC6
  }

  //return height of suspension to dashboard(?)
  //scrapped code. not working
  /*
  void returnHeight(){
  void send_asc2
  {
    suspension_command2_data[2] = suspension_command2_data[2] & 0xF;
    stat = canWrite(hnd4, suspension_command2_id, suspension_command2_data, suspension_command2_dlc, suspension_command2_flag);

    CheckStat(stat);
  }

  void fake_asc1()  // Thread used to control the transmission
{
    long Drive_ID = 0x18FE5947; //0x4FF5527 Look up how to give binary value 00100111111110101010100100111
    unsigned int Drive_DL = 3; //3 Bytes ??
    unsigned int Drive_FLAG = canMSG_EXT; //Indicates extended ID

    Drive_DATA[1]=10;

    hnd2 = canOpenChannel(1,  canOPEN_REQUIRE_EXTENDED);    // Open channel for speed control
    stat=canSetBusParams(hnd2, canBITRATE_250K, 0, 0, 0, 0, 0); // Set bus parameters
        CheckStat(stat);
    stat=canSetBusOutputControl(hnd2, canDRIVER_NORMAL);        // set driver type normal
        CheckStat(stat);
    stat=canBusOn(hnd2);                                        // take channel on bus and start reading messages
        CheckStat(stat);

    while (SpeedOn){


    stat=canWrite(hnd2, Drive_ID, Drive_DATA, Drive_DL, Drive_FLAG);

    //cout << "Data = " << Drive_DATA << endl;
    CheckStat(stat);
    this_thread::yield();
    this_thread::sleep_for (chrono::milliseconds(10));
    }

    stat = canBusOff(hnd2); // Take channel offline
    CheckStat(stat);
    canClose(hnd2);
}
*/

  int main()
  {
    unsigned char * height = new unsigned char[2];
    unsigned char * newHeight = new unsigned char[2];
    int i = 0;
    int j = 0;
    while(1){

     do{
        switch(getch()) { // the real value

        case 72:
           // cout << "arrow up" << endl;
            i++;
            (newHeight[0] >> i) & 0x01;
            (newHeight[1] >> i) & 0x01;
            height[0] = height[0] & newHeight[0];
            height[1] = height[1] & newHeight[1];
            break;
        case 80:
           // cout << "arrow down" << endl;
            j++;
            (newHeight[0] << j) & 0x01;
            (newHeight[1] << j) & 0x01;
            height[0] = height[0] & newHeight[0];
            height[1] = height[1] & newHeight[0];
            break;
        }
     }

    canInitializeLibrary(); //Initialize driver

    SpeedOn = true;
    std::thread t2 (setHeight(height));
    std::thread t1 (setPresetLevel);
   // std::thread t3 (returnHeight);

    //printf("Byte 1: %d ", Drive_DATA[1]);

    t1.join();
    t2.join();
  //  t3.join();

    return 0;
    }
  }
  // ASC6 to new angle according to ASC3
//  unsigned ASC6NewAnglemessage = new unsigned char[8];
//
//
//  //Send the ASC2 command message with the �Nominal Level Request Rear Axle� field set to �N/A�
//  unsigned ASC2NAmessage = "00001111";

//
//  //wait for reception of ASC1 Message
//  stat = canRead(handle, &ASC1id, ASC1message, &Command_DL, &Command_Flag, &Command_TIMEOUT);
//    if (ASC1message == "01001111")
//	//set ASC6 command with initial level
//        stat=canWrite(handle, ASC6iid, ASC6imessage,ASC6iCommand_DL,ASC6iCommand_FLAG);
//
//  //Override Height
//  while (1)
//	{
//	  //wait for reception of status message (ASC1)
//	  stat = canRead(handle, &ASC1id, ASC1message, &Command_DL, &Command_Flag, &Command_TIMEOUT);
//	  CheckStat(stat);
//	  // if Nominal Level Rear Axle is not on Preset level
//        while (ASC1message != 01001111)
//		  //send command message (ASC2) with preset level for rear axel
//		  stat= canWrite(handle, &ASC2id, ASC2message, ASC2Command_DL, ASC2Command_FLAG);	// Will need to be sent a specified frequency
//		  CheckStat(stat);
//	  //monitor for message (ASC3)
//      stat = canRead(handle, &ASC3id, ASC3message, &ASC3Command_DL, &ASC3Command_Flag,&Command_TIMEOUT);
//      CheckStat(stat);
//      // DECODE MESSAGE TO GET ANGLE
//      // SEND ASC6 MESSAGE TO NEW ANGLE BASED ON DECODED MESSAGE
//      stat= canWrite(handle, &ASC6id, ASC6NewAnglemessage, ASC6Command_DL, ASC6Command_FLAG);
//	  CheckStat(stat);
//	}
//  //if status mesage (ASC1) is N/A
//  //return height control to cabs dashboard
//
//
//  stat = canRead(handle, &ASC1id, ASC1message, &Command_DL, &Command_Flag, &Command_TIMEOUT);
//  while(ASC1message != "00001111")
//    //send ASC2 mesage with nominal level request rear axle set to N/A
//    stat= canWrite(handle, &ASC2id, ASC2NAmessage, ASC2Command_DL, ASC2Command_FLAG);
//    CheckStat(stat);
//}
