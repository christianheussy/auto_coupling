#include "path.h"

int path(double dist_grad, double y[RES], double d, double t1, double t2)
{
	int i;
	double a_max, b_max, x_b, y_b, x_f, y_f, b, a;
	a_max = 1 / (2 * RMIN);
	b_max = abs(sqrt((double)3) / (6 * pow((double) RMIN, 2)));
	x_b = d*cos(t1);
	y_b = d*sin(t1);
	x_f = x_b + L*cos(t2);
	y_f = y_b + L*sin(t2);
	b = (y_b - y_f*pow(x_b, 2) / pow(x_f, 2)) * 1 / (pow(x_b, 3) - x_f*pow(x_b, 2));
	a = (y_f - b*pow(x_f, 3)) / pow(x_f, 2);

	if (a > a_max || b > b_max){ //exit with return value 0 signifying that path is impossible
		return 0;
	}
	else{
		for (i = 0; i < RES; i++)
		{
			y[i] = a*pow(i*dist_grad, 2) + b*pow(i*dist_grad, 3);
		}
		return 1;
	}
}
