#include <cmath>
#include <stdlib.h>

int path(float& a, float& b, float d, float t1, float t2)
{
    extern float RMIN;
    extern float L;
	float a_max, b_max, x_b, y_b, x_f, y_f;
	a_max = 1.0 / (2.0 * RMIN);
	b_max = abs(sqrt((float)3.0) / (6.0 * pow((float)RMIN, 2)));

	x_f = d*cosf(t1);
	y_f = d*sinf(t1);
	x_b = x_f - L*cosf(t2);
	y_b = y_f - L*sinf(t2);
	b = (y_f*pow(x_b, 2)/pow(x_f, 2) - y_b)/(x_f*pow(x_b, 2) - pow(x_b, 3));
	a = (y_f - b*pow(x_f, 3)) / pow(x_f, 2);

	if (a > a_max || b > b_max){ //exit with return value 0 signifying that path is impossible
		return 0;
	}
	else{
		return 1;
	}
}
