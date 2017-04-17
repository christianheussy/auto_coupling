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

using namespace std;
// Global variable to avoid passing to each function.
static LeddarHandle gHandle=NULL;
static LeddarHandle lHandle=NULL;

int DETS;
float WID;
float VIA;
//Path Variables
float RES;
float RMIN;
float L;
//Lower LIDAR Variables
float RATE;
float INIT_ANG;
float BEAM;
float THRESH;
int SPEED;
//Debug
int DEBUG;
int SIMPLE;
float OFFSET;

void low_LID(float low_dist[16], int& king_f, float& king_dis, float& king_angle, float& height, float& closest);
void detect(float res[8][3], float top_dist[16]);

int main()
{
    float value;
//Initialize Upper Lidar
    gHandle = LeddarCreate();
    char lAddress[24];
    int i,a,n;
    strcpy(lAddress,"AH45001");
    LeddarConnect( gHandle, lAddress );
    LdDetection lDetections[50];


//initialize Lower LIDAR
    lHandle = LeddarCreate();
    strcpy(lAddress,"FIND ADDRESS");
    LeddarConnect( lHandle, lAddress );

//Set up Communication
    int listenfd = 0,connfd = 0;

    struct sockaddr_in serv_addr;
    string mess,name;
    char sendBuff[200];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
//Main Loop
    while(1){
//Wait for communication
    while(listen(listenfd, 10) == -1){
    }
    connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
//Set Constants
    stringstream iss,deb;
    write(connfd, "give", strlen("give"));
    while(n = read(connfd,sendBuff,sizeof(sendBuff)))
    {
        sendBuff[n] = 0;
        mess = sendBuff;
        iss << mess;
        iss >> name;
        iss >> value;
        iss.str(string());
        deb << mess << endl;

        if(name == "DETS")
            DETS = value;
        else if(name == "WID")
            WID = value;
        else if(name == "VIA")
            VIA = value*acosf((float)-1)/(180.0);
        else if(name == "RES")
            RES = value;
        else if(name == "RMIN")
            RMIN = value;
        else if(name == "L")
            L = value;
        else if(name == "RATE")
            RATE = value;
        else if(name == "INIT_ANG")
            INIT_ANG = value*acosf((float)-1)/(180.0);
        else if(name == "BEAM")
            BEAM = value*acosf((float)-1)/(180.0);
        else if(name == "THRESH")
            THRESH = value;
        else if(name == "SPEED")
            SPEED = value;
        else if(name == "DEBUG")
            DEBUG = value;
        else if(name == "SIMPLE")
            SIMPLE = value;
    }
    OFFSET = ((float)SPEED /3600)*(1/RATE)*tanf(INIT_ANG);
    if(DEBUG == 1){
        while(deb >> mess){
            strcpy(sendBuff, mess.c_str());
            write(connfd, sendBuff, strlen(sendBuff));
        }
    }
    deb.str(string());

//Start Reading Data
    float low_dist[16] = {0};
    float top_dist[16] = {0};
    float res[8][3] = {0};
    int king_f = 0;
    float king_dis,king_angle,height, closest;
    float choice,choose;

    LeddarStartDataTransfer( gHandle, LDDL_DETECTIONS );
    LeddarStartDataTransfer( lHandle, LDDL_DETECTIONS );

    if(SIMPLE == 0){
        n = read(connfd,sendBuff,sizeof(sendBuff));
        sendBuff[n] = 0;
        mess = sendBuff;
        iss.str(mess);
        iss >> choice;
        iss.str(string());
    }


    while(n = read(connfd,sendBuff,sizeof(sendBuff)))
    {
        sendBuff[n] = 0;
        mess = sendBuff;

        if(mess == "data"){
//Get Upper Distances
            LeddarGetDetections(gHandle, lDetections, ARRAY_LEN( lDetections ));
            for(i = 0; i < 16; i++)
            {
                if(DEBUG == 1)
                    iss << setprecision(3) << lDetections[i].mDistance << " ";
                top_dist[i] = lDetections[i].mDistance;
            }
            if(DEBUG == 1){
                mess = iss.str();
                strcpy(sendBuff, mess.c_str());
                write(connfd, sendBuff, strlen(sendBuff));
                iss.str(string());
            }

//Get Lower Distances
            LeddarGetDetections(lHandle, lDetections, ARRAY_LEN( lDetections ));
            for(i = 0; i < 16; i++)
            {
                if(DEBUG == 1)
                    iss << setprecision(3) << lDetections[i].mDistance << " ";
                low_dist[i] = lDetections[i].mDistance;
            }
            if(DEBUG == 1){
                mess = iss.str();
                strcpy(sendBuff, mess.c_str());
                write(connfd, sendBuff, strlen(sendBuff));
                iss.str(string());
            }
             //Calculate parameters
low_LID(low_dist, king_f, king_dis, king_angle, height, closest);
detect(res, top_dist);
            iss.str(string());
            //Send data
            if(DEBUG == 1){
                for(i = 0; i < 8; i++){
                    iss << setprecision(3) <<  res[i][0] << " " << res[i][1] << " " << res[i][2];
                    mess = iss.str();
                    strcpy(sendBuff, mess.c_str());
                    write(connfd, sendBuff, strlen(sendBuff));
                    iss.str(string());
                }
                iss << setprecision(3) << king_f << " " << king_dis << " " << king_angle << " " << height << " " << closest;
                mess = iss.str();
                strcpy(sendBuff, mess.c_str());
                write(connfd, sendBuff, strlen(sendBuff));
                iss.str(string());
            }
            a = -1;
            choose = VIA/2.0;
            if(SIMPLE == 0){
                for(i = 0; i < 8; i++){
                   if(abs(choice - res[i][2] + res[i][1]) < choose){
                        choose = choice - res[i][2] + res[i][1];
                        if(abs(choose) < 2*BEAM)
                            a = i;
                   }
                }
            }else{
                 for(i = 0; i < 8; i++){
                   if(abs(res[i][2] - res[i][1]) < choose){
                        choose = choice - res[i][2] + res[i][1];
                        a = i;
                   }
                }
            }

            if(a == -1){
                a = 8;
                res[a][0] = 0;
                res[a][1] = 0;
                res[a][2] = 0;
            }

            if(king_f == 1){
                res[a][0] = king_dis;
                res[a][1] = king_angle;
                king_f = 2;
            }

            iss << setprecision(3) << res[a][0] << " " << res[a][1] << " " << res[a][2]
                << " " << king_f << " " << height << " " << closest;
            mess = iss.str();
            strcpy(sendBuff, mess.c_str());
            write(connfd, sendBuff, strlen(sendBuff));
            iss.str(string());
        }else if(mess == "end"){
        break;
        }
    }
    close(connfd);
    LeddarStopDataTransfer(gHandle);
    LeddarStopDataTransfer(lHandle);
    }
    return 0;
}
