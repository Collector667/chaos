//
// Created by Муслихиддин Махмудов on 29.09.2026.
//
module;
#include <Eigen/Dense>

export module MidPointMethod;

using State = Eigen::Vector3d;


inline constexpr auto midPointStep = [](auto dx, auto dy, auto dz,
                                        double& x, double& y, double& z, double h)
{
    double k1x = dx(x, y, z);
    double k1y = dy(x, y, z);
    double k1z = dz(x, y, z);

    double k2x = dx(x + h / 2.0 * k1x, y + h / 2.0 * k1y, z + h / 2.0 * k1z);
    double k2y = dy(x + h / 2.0 * k1x, y + h / 2.0 * k1y, z + h / 2.0 * k1z);
    double k2z = dz(x + h / 2.0 * k1x, y + h / 2.0 * k1y, z + h / 2.0 * k1z);

    x += h * k2x;
    y += h * k2y;
    z += h * k2z;
};

export inline constexpr auto midPoint = [](const State& s, double dt, auto sys) -> State {
    State k1 = sys(s);
    State k2 = sys(s+0.5*dt*k1);
    return s+dt*k2;
};





