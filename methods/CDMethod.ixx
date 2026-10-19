//
// Created by Муслихиддин Махмудов on 29.09.2026.
//

module;
#include <Eigen/Dense>

export module CDMethod;

using State = Eigen::Vector3d;

// Полушаговая (палиндромная) схема x, y, z, z, y, x.
// Полушаги x и y — явные, второй полушаг по z — неявный (тип "Cramer").
// Поэтому системе нужны коэффициенты: sys должен предоставлять поле coefs
// (см. CaseISystem в модуле system). Неявная формула ниже предполагает
// структуру Sprott Case I:  dz/dt = coefs[3]*x + coefs[4]*y^2 + coefs[5]*z.
// Композиция "явный полушаг + неявный полушаг" по z даёт множитель
// (1 + h*f)/(1 - h*f) (аппроксимация Паде (1,1)), то есть 2-й порядок
// по z и A-устойчивость при f < 0 (h = dt/2).
export inline constexpr auto CD = [](const State& s, double dt, const auto& sys) -> State
{
    State cur = s;
    auto dx = [&](const State& p) { return sys(p)[0]; };
    auto dy = [&](const State& p) { return sys(p)[1]; };
    auto dz = [&](const State& p) { return sys(p)[2]; };

    cur[0] += dx(cur) * dt * 0.5;
    cur[1] += dy(cur) * dt * 0.5;
    cur[2] += dz(cur) * dt * 0.5;

    // Неявный полушаг по z: z_new = (z + h*(c3*x + c4*y^2)) / (1 - h*c5),  h = dt/2
    const auto& c = sys.coefs;
    cur[2] = (cur[2] + 0.5 * dt * (c[3] * cur[0] + c[4] * cur[1] * cur[1]))
             / (1.0 - c[5] * dt * 0.5);

    cur[1] += dy(cur) * dt * 0.5;
    cur[0] += dx(cur) * dt * 0.5;
    return cur;
};
