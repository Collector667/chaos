//
// Created by Муслихиддин Махмудов on 29.09.2026.
//
module;
#include <Eigen/Dense>
export module EulerCramerMethod;


using State = Eigen::Vector3d;


export inline constexpr auto eulerCromerStep = [](auto dx, auto dy, auto dz,
                                           double& x, double& y, double& z, double h)
{
    x = x + h * dx(x, y, z);
    y = y + h * dy(x, y, z);
    z = z + h * dz(x, y, z);
};

export inline constexpr auto eulerCramer = [](const State& s, double dt, auto sys) -> State {
    State k1 = sys(s);
    State tempS = s;

    for (int i = 0; i< s.size(); i++) {
        k1 = sys(tempS);
        tempS[i] += k1[i]*dt;
    }
    return tempS;
};


