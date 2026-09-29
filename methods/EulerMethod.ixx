//
// Created by Муслихиддин Махмудов on 29.09.2026.
//
module;
#include <Eigen/Dense>


export module EulerMethod;
using State = Eigen::Vector3d;

export inline constexpr auto eulerStep = [](auto dx, auto dy, auto dz,
                                     double& x, double& y, double& z, double h)
{
    double x_new = x + h * dx(x, y, z);
    double y_new = y + h * dy(x, y, z);
    double z_new = z + h * dz(x, y, z);
    x = x_new;
    y = y_new;
    z = z_new;
};



export inline constexpr auto euler = [](const State& s, double dt, auto sys) -> State {
    return s+dt*sys(s);
};



