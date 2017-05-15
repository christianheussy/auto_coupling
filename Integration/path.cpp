#include "path.h"


int path(float& a, float& b, float d, float t1, float t2)
{

	extern float RMIN;
	extern float L;
	//float L = 2.0;

	float x_off, x_fwheel, y_fwheel, x_cam, y_cam;
	x_cam = d*cosf(t1);
	y_cam = d*sinf(t1);
	x_fwheel = x_cam - L*cosf(t2);
	y_fwheel = y_cam - L*sinf(t2);
	b = (y_fwheel - y_cam*pow(x_fwheel, 2) / pow(x_cam, 2)) * 1 / (pow(x_fwheel, 3) - x_cam*pow(x_fwheel, 2));
	a = (y_cam - b*pow(x_cam, 3)) / pow(x_cam, 2);
	//b = (x_fwheel*tanf(t2) - 2*y_fwheel)/-pow(x_fwheel,3);
	//a = (y_fwheel - b*pow(x_fwheel,3))/pow(x_fwheel,2);
	
	
	//x_off = (1 / RMIN - 2 * a)*(1 / (6 * b));
	//if (x_off > 0 && x_off < x_cam){ //exit with return value 0 signifying that path is impossible
	if ((abs(a) < 1 / RMIN) && (abs(b) < 1/(RMIN*RMIN))){
		return 1;
	}
	else{
		return 0;
	}
}
