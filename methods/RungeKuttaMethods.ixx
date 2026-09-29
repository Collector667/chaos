//
// Created by Муслихиддин Махмудов on 29.09.2026.
//
module;
#include <Eigen/Dense>

export module RungeKuttaMethods;


using State = Eigen::Vector3d;

export inline constexpr auto rk4 = [](const State& s, double dt, auto sys) -> State {
    State k1 = sys(s);
    State k2 = sys(s+0.5*dt*k1);
    State k3 = sys(s+0.5*dt*k2);
    State k4 = sys(s+dt*k3);

    return s+(dt/6.0)*(k1+2.0*k2+2.0*k3+k4);
};
