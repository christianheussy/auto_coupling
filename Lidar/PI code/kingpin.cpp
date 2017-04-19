#include <iostream>
#include <cmath>

extern float INIT_ANG;
extern float VIA;
extern float BEAM;
extern float THRESH;
extern int SPEED;
extern float OFFSET;

void low_LID(float distances[16], int& kingpin, float& dis, float& angle, float& height, float& closest)
{
	int i, maxind,minind;
	float diff[15];
	float maxdif = 0;
	float mindif = 0;


	for (i = 0; i < 15; i++)
	{
		diff[i] = distances[i + 1] - distances[i];
	}
	for (i = 0; i < 15; i++)
	{
		if (diff[i] > maxdif){
			maxdif = diff[i];
			maxind = i;
		}
		if (diff[i] < mindif){
			mindif = diff[i];
			minind = i;
		}
	}
	if ((minind - maxind) < 3 && (minind - maxind) > 0 && kingpin != 2)
		kingpin = 1;
	dis = ((distances[minind] + distances[maxind]) / 2)*cos(INIT_ANG);
	angle = VIA/2.0 - BEAM*((minind + maxind) / 2);


	//Height
	float hold = 0;
	float heights[16];
	for (i = 0; i < 16; i++){
		heights[i] = distances[i] * cosf(VIA/2.0 - i*BEAM) * sinf(INIT_ANG);
	}
    mindif = 20.0;//used to find closest object
	if (kingpin == 0){
		for(i = 0; i < 16; i++)
        {
            if((heights[i]-OFFSET) < hold)
                hold = heights[i]-OFFSET;
            if(distances[i] < mindif)
                mindif = distances[i];
        }
        closest = mindif;
        height = hold;
	}else if(kingpin == 1){
		height = dis*cosf(angle) * sinf(INIT_ANG);
	}
}
