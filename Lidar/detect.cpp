#include "detect.h"
#include "path.h"

using namespace std;

void detect(double result[DETS/2][3], double dist[DETS])
{

	//int dist[DETS];
	int NUM = DETS - 1;
	double dydx[NUM] = { 0 }, dydx_2[DETS] = { 0 }, dydx_2s[DETS] = { 0 };
	double xx[DETS] = { 0 }, yy[DETS] = { 0 }, ddx[DETS / 2][2] = { 0 }, ddy[DETS / 2][2] = { 0 }, slops[DETS / 2] = { 0 }, D[DETS / 2] = { 0 };
	int i;
	double m1, m2, b1, b2, Tn, L1, L2;
	int c = 0, z = 0, b = 1;
	double x[DETS]; //= { 1, 2, 3, 2, 1, 2, 3, 2, 1, 2, 3, 2, 1, 2, 3, 2 };
	double y[DETS]; // = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

	for (i = 0; i <= NUM; i++)
	{
	//create (x,y) points from distance data
		x[i] = sin(-VIA / 2 + i*(double(VIA) / double(NUM)))*dist[i];
		y[i] = cos(VIA / 2 - i*(double(VIA) / double(NUM)))*dist[i];
	}
	//calculate the slopes between points
	for (i = 0; i < NUM; i++)
	{
		dydx[i] = ((y[i + 1] - y[i])/(x[i + 1] - x[i]));
	}
	//calculate the second derivatives and round to nearest integer
	for (i = 0; i < NUM-1; i++)
	{
		dydx_2[i+1] = (dydx[i+1] - dydx[i]) / (x[i+2] - x[i+1]);
		dydx_2[i+1] = round(dydx_2[i+1]);
	}
	//set values at the edges of detection equal to their adjacent values
	dydx_2[0] = dydx_2[1];
	dydx_2[NUM] = dydx_2[NUM - 1];
	//set values that are not zero equal to 1, so that places where we see a line will have 0 and others will have 1
	for (i = 0; i < DETS; i++)
	{
		if (!(dydx_2[i] == 0))
		{
			dydx_2[i] = 1;
		}
	}
	//adjust the dydx_2 array so that the detected lines include the line endpoints (since at the endpoint the second derivative
	//will not be 0, but it is still part of the line).
	for (i = 0; i < DETS; i++)
	{
		if (i == 0)
		{
			dydx_2s[i] = dydx_2[i] * dydx_2[i + 1];
		}
		else if (i == NUM)
		{
			dydx_2s[i] = dydx_2[i] * dydx_2[i - 1];
		}
		else
		{
			dydx_2s[i] = dydx_2[i] * dydx_2[i + 1] * dydx_2[i - 1];
		}
	}
	//use the adjusted array dydx_2s to filter out the x and y coordinates of lines
	for (i = 0; i < DETS; i++)
	{
		xx[i] = x[i] - x[i] * dydx_2s[i];
		yy[i] = y[i] - y[i] * dydx_2s[i];
	}
	//pick out the start points and end points of lines and put them into separate rows (so ddx[1][0]
	//will be the x coordinate of the beginning fo the second line in our detection)
	for (i = 0; i <= NUM; i++)
	{
		if (b)
		{
			if (!(xx[i] == 0))
			{
				ddx[c][z] = xx[i];
				ddy[c][z] = yy[i];
				z = 1;
				b = !b;
			}
		}
		else
		{
			if (xx[i] == 0)
			{
				//check if any lines are adjacent
				if ((i < DETS - 2) && (dydx_2[i] == 1) && ((dydx_2[i + 1] == 0) || (dydx_2[i + 2] == 0)))
				{
					//calculate slopes and intercepts of adjacent lines
					m1 = (y[i] - ddy[c][z - 1]) / (x[i] - ddx[c][z - 1]);
					m2 = (y[i + 2] - y[i + 1]) / (x[i + 2] - x[i + 1]);
					b1 = y[i] - m1*x[i];
					b2 = y[i + 2] - m2*x[i + 2];
					//if lines are not approximately perpendicular then use their x coordinates
					if(round(-10*m1*m2)/10 > 2 || round(-10*m1*m2)/10 < .7){
                        ddx[c][z] = x[i];
                        ddy[c][z] = y[i];
                        c = c + 1;
                        ddx[c][z-1] = x[i+1];
                        ddy[c][z-1] = y[i+1];
                    }else{
					//if lines are approximately perpendicular calculate the end and start point from the intersection
					ddx[c][z] = (b2 - b1) / (m1 - m2);
					ddy[c][z] = m1*ddx[c][z] + b1;
					c = c + 1;
					ddx[c][z - 1] = ddx[c - 1][z];
					ddy[c][z - 1] = ddy[c - 1][z];
					}
                continue;
				}
				ddx[c][z] = xx[i - 1];
				ddy[c][z] = yy[i - 1];
				z = 0;
				c = c + 1;
				b = !b;
			}
			else if (i == NUM)
			{
				ddx[c][z] = xx[i];
				ddy[c][z] = yy[i];
			}
		}
	}
	//calculate the slopes and distances of detected lines
	for (i = 0; i < (DETS / 2); i++)
	{
		if (!(ddx[i][0] == ddx[i][1]))
		{
			slops[i] = (ddy[i][1] - ddy[i][0]) / (ddx[i][1] - ddx[i][0]);
		}
		D[i] = sqrt(pow((ddx[i][1] - ddx[i][0]), 2) + pow((ddy[i][1] - ddy[i][0]), 2));
	}
	//if any lines are within 5% of trailer width assume a detection and calculate theta 1 and 2, then pass the parameters to result
	for (i = 0; i < (DETS / 2); i++)
	{
		if ((D[i] > (.95*WID)) && (D[i] < 1.05*WID))
		{
			L2 = sqrt(pow(ddx[i][0], 2) + pow(ddy[i][0], 2));
			L1 = sqrt(pow(ddx[i][1], 2) + pow(ddy[i][1], 2));
			Tn = acos((pow((double)WID, 2) + pow(L1, 2) - pow(L2, 2)) / (2 * L1*WID));
			result[i][0] = sqrt(pow((double)WID, 2) / 2 + pow(L1, 2) - L1*WID*cos(Tn));
			result[i][1] = acos((double)-1) / 2 - asin(sin(Tn)*L1 / result[i][0]);
			if (slops[i] == 0)
			{
				result[i][2] = 0;
			}
			else
			{
				result[i][2] = atan(-1 / slops[i]) - acos((double)-1) / 2;
			}
		}
	}


}
