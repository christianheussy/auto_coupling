#include "config.h"

using namespace std;

int main()
{
//Initialize Connection With RaspberryPi
int sockfd = 0,n = 0, i = 0, kp_flag = 0;
char recvBuff[200];
struct sockaddr_in serv_addr;
float dis_LID, height_LID, closest, t1_LID, t2_LID, choice;
string mess;
stringstream iss;
iss.str("");
iss.clear();

memset(recvBuff, '0' ,sizeof(recvBuff));
//int coup_flag;

//Initialize Constants
config();

while((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
{
sockfd = 0;
sockfd = socket(AF_INET, SOCK_STREAM, 0);
}

serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(5000);
serv_addr.sin_addr.s_addr = inet_addr("10.0.0.20");

while(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
{
  close(sockfd);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(5000);
  serv_addr.sin_addr.s_addr = inet_addr("10.0.0.20");
}
//Send Constants
while(1)
{
    n = (read(sockfd, recvBuff, sizeof(recvBuff)));
    recvBuff[n] = 0;
    mess = recvBuff;
    if(mess == "give")
    {
        ifstream myfile;
        myfile.open("configer");
        while(getline(myfile,mess,'\0'))
        {
            strcpy(recvBuff,mess.c_str());
            write(sockfd,recvBuff,sizeof(recvBuff));
            //cout << mess.c_str() << endl;
        }
        myfile.close();
        break;
    }
}
//Receive echo of constants
if(DEBUG == 1){
    cout << "Constants:" << endl;
    n = read(sockfd, recvBuff, sizeof(recvBuff));
    recvBuff[n] = 0;
    iss.str(recvBuff);
    cout << iss.str() << endl;
    iss.str("");
    iss.clear();
}
//Send Trailer Choice
if(SIMPLE == 0){
choice = VIA/2.0;
write(sockfd,&choice,sizeof(choice));
}

//Read Data from RaspberryPi

write(sockfd,"data",strlen("data"));

if(DEBUG == 1){
    cout << "Upper LIDAR Readings:" << endl;
    n = read(sockfd, recvBuff, sizeof(recvBuff));
    recvBuff[n] = 0;
    iss.str(recvBuff);
    cout << iss.str() << endl;

    cout << "Lower LIDAR Readings:" << endl;
    iss.str("");
    iss.clear();
    n = read(sockfd, recvBuff, sizeof(recvBuff));
    recvBuff[n] = 0;
    iss.str(recvBuff);
    cout << iss.str() << endl;

    cout << "detection results:" << endl;
    iss.str("");
    iss.clear();
    for(i = 0; i < 8; i++)
    {
        n = read(sockfd, recvBuff, sizeof(recvBuff));
        recvBuff[n] = 0;
        iss.str(recvBuff);
        cout << iss.str() << endl;
        iss.str("");
        iss.clear();
    }

    cout << "k_f   k_d   k_a   he   cl" << endl;
    n = read(sockfd, recvBuff, sizeof(recvBuff));
    recvBuff[n] = 0;
    iss.str(recvBuff);
    cout << iss.str() << endl;
    iss.str("");
    iss.clear();
}
n = read(sockfd, recvBuff, sizeof(recvBuff));
recvBuff[n] = 0;
iss.str(recvBuff);
iss >> dis_LID >> t1_LID >> t2_LID >> kp_flag >> height_LID >> closest;
//   iss >> coup_flag;
iss.str("");
iss.clear();

return 0;
}
