#include "config.h"

using namespace std;

int main()
{
//Initialize Connection With RaspberryPi
  int sockfd = 0,n = 0, i = 0;
  char recvBuff[300];
  struct sockaddr_in serv_addr;
  string mess;
  memset(recvBuff, '0' ,sizeof(recvBuff));
  float dis_LID;
  float t1_LID;
  float t2_LID;
  int kp_flag;
  float height_LID;
  float closest;
//int coup_flag;
//Initialize Constants
  config();

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
      printf("\n Error : Could not create socket \n");
      return 1;
    }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(5000);
  serv_addr.sin_addr.s_addr = inet_addr("10.0.0.20");

  while(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
      printf("\n Error : Connect Failed \n");
      close(sockfd);
    }

    //Send Constants
    while((n = read(sockfd, recvBuff, sizeof(recvBuff))))
    {
        recvBuff[n] = 0;
        mess = recvBuff;

        if(mess == "give")
        {
            ifstream myfile;
            myfile.open("configer");

            while(getline(myfile,mess))
            {
                strcpy(recvBuff,mess.c_str());
                write(sockfd,recvBuff,sizeof(recvBuff));
            }
            myfile.close();
            break;
        }else{
            recvBuff[0] = 0;
        }
    }
    //Receive echo of constants
    if(DEBUG == 1){
        cout << "Constants:" << endl;
        while((n = read(sockfd, recvBuff, sizeof(recvBuff))))
        {
            recvBuff[n] = 0;
            mess = recvBuff;
            cout << mess << endl;
        }
    }



    if(SIMPLE == 0){
//take out theta_1
    float theta_1 = VIA/2.0;
    mess = theta_1;
    strcpy(recvBuff,mess.c_str());
    write(sockfd,recvBuff,sizeof(recvBuff));
    }

//Read Data from RaspberryPi
    write(sockfd,"data",sizeof("data"));
    if(DEBUG == 1){
        cout << "Upper LIDAR Readings:" << endl;
        n = read(sockfd, recvBuff, sizeof(recvBuff));
        recvBuff[n] = 0;
        mess = recvBuff;
        cout << mess << endl;
        cout << "Lower LIDAR Readings:" << endl;
        n = read(sockfd, recvBuff, sizeof(recvBuff));
        recvBuff[n] = 0;
        mess = recvBuff;
        cout << mess << endl;
        cout << "detection results:" << endl;
        for(i = 0; i < 8; i++)
        {
            n = read(sockfd, recvBuff, sizeof(recvBuff));
            recvBuff[n] = 0;
            mess = recvBuff;
            cout << mess << endl;
        }
        cout << "k_f   k_d   k_a   he   cl" << endl;
        n = read(sockfd, recvBuff, sizeof(recvBuff));
        recvBuff[n] = 0;
        mess = recvBuff;
        cout << mess << endl;
    }
    n = read(sockfd, recvBuff, sizeof(recvBuff));
    recvBuff[n] = 0;
    istringstream iss(recvBuff);
    iss >> dis_LID;
    iss >> t1_LID;
    iss >> t2_LID;
    iss >> kp_flag;
    iss >> height_LID;
    iss >> closest;
 //   iss >> coup_flag;

}
