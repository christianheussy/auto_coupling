#include "path.h"


int path(float& a, float& b, float d, float t1, float t2)
{
	float RES = 1000.0;
	float RMIN = 7.2;
	float L = 2.0;

	float a_max, b_max, x_b, y_b, x_f, y_f;
	a_max = 1 / (2 * RMIN);
	b_max = abs(sqrt((float)3) / (6 * pow((float)RMIN, 2)));


	if (a > a_max || b > b_max){ //exit with return value 0 signifying that path is impossible
		return 0;
	}
	else{
		x_f = d*cosf(t1);
		y_f = d*sinf(t1);
		x_b = x_f - L*cosf(t2);
		y_b = y_f - L*sinf(t2);
		b = (y_b - y_f*pow(x_b, 2) / pow(x_f, 2)) * 1 / (pow(x_b, 3) - x_f*pow(x_b, 2));
		a = (y_f - b*pow(x_f, 3)) / pow(x_f, 2);
		return 1;
	}
}
