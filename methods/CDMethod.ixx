//
// Created by Муслихиддин Махмудов on 29.09.2026.
//

module;
#include <Eigen/Dense>

export module CDMethod;

using State = Eigen::Vector3d;

export inline constexpr auto CD = [](const State& s, double dt, auto sys) -> State
{
    State cur = s;
    auto dx = [&](const State& p) { return sys(p)[0]; };
    auto dy = [&](const State& p) { return sys(p)[1]; };
    auto dz = [&](const State& p) { return sys(p)[2]; };

    cur[0] += dx(cur) * dt * 0.5;
    cur[1] += dy(cur) * dt * 0.5;
    cur[2] += dz(cur) * dt * 0.5;
    cur[2] += dz(cur) * dt * 0.5;
    cur[1] += dy(cur) * dt * 0.5;
    cur[0] += dx(cur) * dt * 0.5;
    return cur;
};
