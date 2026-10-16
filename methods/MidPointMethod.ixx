//
// Created by Муслихиддин Махмудов on 29.09.2026.
//
module;
#include <Eigen/Dense>

export module MidPointMethod;

using State = Eigen::Vector3d;

export inline constexpr auto midPoint = [](const State& s, double dt, auto sys) -> State
{
    State k1 = sys(s);
    State k2 = sys(s + 0.5 * dt * k1);
    return s + dt * k2;
};
