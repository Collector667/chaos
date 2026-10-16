//
// Created by Муслихиддин Махмудов on 29.09.2026.
//
module;
#include <Eigen/Dense>
export module EulerCramerMethod;

using State = Eigen::Vector3d;

export inline constexpr auto eulerCramer = [](State& s, double dt, auto dx, auto dy, auto dz) -> State
{
    s[0] += dx(s) * dt;
    s[1] += dy(s) * dt;
    s[2] += dz(s) * dt;

    return s;
};
