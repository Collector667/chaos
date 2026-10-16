//
// Created by Муслихиддин Махмудов on 29.09.2026.
//

module;
#include <Eigen/Dense>

export module CDMethod;

using State = Eigen::Vector3d;

export inline constexpr auto CD = [](State& s, double dt, auto dx, auto dy, auto dz) -> State
{
    s[0] += dx(s) * dt * 0.5;
    s[1] += dy(s) * dt * 0.5;
    s[2] += dz(s) * dt * 0.5;

    s[2] += dz(s) * dt * 0.5;
    s[1] += dy(s) * dt * 0.5;
    s[0] += dx(s) * dt * 0.5;

    return s;
};
