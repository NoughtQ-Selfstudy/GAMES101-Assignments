#include<cmath>
#include<Eigen/Core>
#include<Eigen/Dense>
#include<iostream>

using Eigen::Matrix3d;
using Eigen::Vector3d;

double degree2radian(double d) {
    return d / 180 * M_PI;
}

int main() {
    Matrix3d T;
    double alpha = degree2radian(45);
    Vector3d point;

    T(0, 0) = cos(alpha);
    T(0, 1) = -sin(alpha);
    T(0, 2) = 1;
    T(1, 0) = sin(alpha);
    T(1, 1) = cos(alpha);
    T(1, 2) = 2;
    T(2, 0) = 0;
    T(2, 1) = 0;
    T(2, 2) = 1;

    point << 2.0, 1.0, 1.0;

    std::cout << "Location of transformed point: \n";
    std::cout << T * point << std::endl; 

    return 0;
}