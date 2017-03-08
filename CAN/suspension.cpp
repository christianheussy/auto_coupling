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
  long suspension_status1_id = 0x18FE5A47;  //11000111111100101101000100111
  unsigned char * suspension_status1_data = new unsigned char[8];
  unsigned int * suspension_status1_dlc; //Data length
  unsigned int * suspension_status1_flag;
  unsigned long * suspension_status1_time; //Indicates extended ID
  unsigned long suspension_status1_timeout = 50; // Timeout for read wait

  // Create ASC6 initial level command message
  // Range -600 to 600
  // Initial preset: 0
  long suspension_command1_id = 0x668AF47;  //0110011010001010111100100111;
  unsigned char * suspension_command1_data = new unsigned char[8];
  unsigned int suspension_command1_dlc = 8;
  unsigned int suspension_command1_flag = canMSG_EXT;

  // ASC2 command message w/ nominal level request axle set to preset level
  long suspension_command2_id = 0x6692F47; // 0110011010010010111100100111;
  unsigned char * suspension_command2_data = new unsigned char[8];
  unsigned int suspension_command2_dlc = 8; //Data length
  unsigned int suspension_command2_flag = canMSG_EXT; //Indicates extended ID
  unsigned long suspension_command2_timeout = 1000; // Timeout for read wait

  // ASC3 status message
  long suspension_status2_id = 0x18FE5947; //11000111111100101100100100111;
  unsigned char * suspension_status2_data = new unsigned char[8];
  unsigned int suspension_status2_dlc = 8; //Data length
  unsigned int suspension_status2_flag = canMSG_EXT; //Indicates extended ID
  unsigned long suspension_status2_timeout = 1000; // Timeout for read wait

  void setPresetLevel(){

  hnd4 = canOpenChannel(0,  canOPEN_REQUIRE_EXTENDED);
  stat=canBusOn(hnd4);
  CheckStat(stat);
  unsigned long timestamp;
  suspension_command1_data[1] = suspension_command1_data[1] & 0x04; //message ASC2 set to preset level

  stat=canReadWait(hnd4, &suspension_status1_id, suspension_status1_data, suspension_status1_dlc, suspension_status1_flag, suspension_status1_time,suspension_status1_timeout);
    if (CheckStat < 0)
        printf("error");
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
    stat=canReadWait(hnd4, &suspension_status2_id, suspension_status2_data, &suspension_status2_dlc, &suspension_status2_flag, suspension_status1_time,suspension_status2_timeout);
    if (CheckStat < 0)
        printf("error");
    suspensionHeight[0] = suspension_status2_data[4] & 0xFF;
    suspensionHeight[1] = suspension_status2_data[5] & 0xFF;

    //for(int i = 4, i < 6, i++)

    suspension_command2_data[5] = suspensionHeight[0] & 0xFF;
    suspension_command2_data[6] = suspensionHeight[1] & 0xFF;

    //convert int height into char
    stat = canWrite(hnd4,suspension_command2_id, suspension_command2_data, suspension_command2_dlc, suspension_command2_flag);
    if (CheckStat(stat) < 0)
        printf("error");
    else
        cout << "successful write" << endl;
    //decode message thats binary to a readable angle
    //set angle with ASC6
  }

/*
  void returnHeight(){
  void send_asc2
  {
    suspension_command2_data[2] = suspension_command2_data[2] & 0xF;
    stat = canWrite(hnd4, suspension_command2_id, suspension_command2_data, suspension_command2_dlc, suspension_command2_flag);

    CheckStat(stat);
  }
*/

  int main()
  {
    unsigned char * height = new unsigned char[2];
    unsigned char * newHeight = new unsigned char[2];
    int i = 0;
    int j = 0;
    int c = 0;

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
     }while(c<1);
    void setHeight();
    void setPresetLeve();

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
