#pragma once // This file is only included once, even if multiple files #include it

#include <opencv2/core/core.hpp>
#include <zed/Camera.hpp>

bool adjustCrosshairsByInput(int &x, int &y, int nRows, int nCols);
void drawCrosshairsInMat(cv::Mat &mat, int x, int y);
void coordinateGrab(cv::Mat contours, int x, int y, int& left, int& right, int& x_center, int& y_center);
void distanceGrab(float& l1, float& l2, int& left, int& right, int& y_center, sl::zed::Mat distancevalue);
void pathInputCalculations_Camera(float left_dist, float right_dist, float& center_dist, float& theta_1, float& theta_2, int leftEdgeCoord, int rightEdgeCoord);
