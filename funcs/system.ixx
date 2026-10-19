module;

#include <array>
#include <cmath>

#include <Eigen/Dense>

export module system;

// Тип состояния системы (общий для всех модулей проекта)
export using State = Eigen::Vector3d;

// Sprott Case I:
//   dx/dt = a*y
//   dy/dt = b*x + c*z
//   dz/dt = d*x + e*y^2 + f*z
// coefs = {a, b, c, d, e, f}
export inline auto CaseI = [](const State& s,
                              const std::array<double, 6>& coefs = {-0.2, 1.0, 1.0, 1.0, 1.0, -1.0}
                              ) -> State
{
    return State{
        coefs[0] * s[1],
        coefs[1] * s[0] + coefs[2] * s[2],
        coefs[3] * s[0] + coefs[4] * s[1] * s[1] + coefs[5] * s[2]
    };
};

// Система вместе со своими коэффициентами.
// Нужна интеграторам, которым требуются параметры правой части
// (CD — неявный полушаг по z). Вызывается как функция и даёт доступ к coefs.
export struct CaseISystem
{
    std::array<double, 6> coefs{-0.2, 1.0, 1.0, 1.0, 1.0, -1.0};

    State operator()(const State& s) const { return CaseI(s, coefs); }
};
