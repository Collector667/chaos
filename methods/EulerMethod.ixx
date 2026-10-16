//
// Created by Муслихиддин Махмудов on 29.09.2026.
//
module;
#include <Eigen/Dense>

export module EulerMethod;
using State = Eigen::Vector3d;

export inline constexpr auto euler = [](const State& s, double dt, auto sys) -> State
{ return s + dt * sys(s); };
