module;

#include <fstream>
#include <filesystem>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <string>
#include <cstddef>
#include <iomanip>

#include <Eigen/Dense>

export module bifurcation;

using State = Eigen::Vector3d;

export inline auto CaseI_bif = [](const State& s,
                                  const std::array<double, 6>& coefs,
                                  double scale = 1.0) -> State
{
    return State{
        coefs[0] * s[1],
        coefs[1] * s[0] + coefs[2] * s[2],
        coefs[3] * s[0] + coefs[4] * s[1] * s[1] + coefs[5] * s[2]
    } * scale;
};

// Строит бифуркационную диаграмму по параметру a = coefs[0],
// ищет локальные максимумы x,y,z на установившемся участке,
// записывает в CSV (a,xmax,ymax,zmax).
// Возвращает кол-во записанных строк или -1 при ошибке.
export int buildBifurcationA(const std::array<double, 6>& coefs_base,
                             double a_min, double a_max, size_t Na,
                             double dt, double T_trans, double T_steady,
                             const std::string& filename,
                             auto integrator)
{
    // Папка csv
    std::filesystem::path csv_dir = std::filesystem::current_path() / "csv";
    if (!std::filesystem::exists(csv_dir))
        std::filesystem::create_directories(csv_dir);

    // Файл
    std::ofstream fout(csv_dir / filename);
    if (!fout.is_open())
    {
        std::cerr << "Не удалось открыть " << (csv_dir / filename) << "\n";
        return -1;
    }

    fout << "a,xmax,ymax,zmax\n";
    fout << std::setprecision(9) << std::fixed;

    // Копия коэффициентов
    std::array<double, 6> coefs = coefs_base;

    // Начальные условия
    State s_state{0.1, 0.0, 0.0};
    std::vector<double> x_s, y_s, z_s;

    int rows = 0;
    double a_cur = a_min;
    double a_step = (a_max - a_min) / (double)(Na - 1);

    for (size_t ia = 0; ia < Na; ++ia)
    {
        // Текущее значение параметра
        coefs[0] = a_cur;

        // Лямбда правой части с текущим a
        auto sys_a = [&](const State& st) -> State
        { return CaseI_bif(st, coefs, 1.0); };

        // 1) Переходный процесс
        s_state = State{0.1, 0.0, 0.0};
        {
            size_t steps_t = (size_t)std::ceil(T_trans / dt);
            for (size_t i = 0; i < steps_t; ++i)
                s_state = integrator(s_state, dt, sys_a);
        }

        // 2) Установившийся участок
        {
            size_t steps_s = (size_t)std::ceil(T_steady / dt);
            x_s.assign(steps_s, 0.0);
            y_s.assign(steps_s, 0.0);
            z_s.assign(steps_s, 0.0);

            for (size_t i = 0; i < steps_s; ++i)
            {
                s_state = integrator(s_state, dt, sys_a);
                x_s[i] = s_state[0];
                y_s[i] = s_state[1];
                z_s[i] = s_state[2];
            }
        }

        // 3) Локальные максимумы x,y,z
        for (size_t i = 1; i + 1 < x_s.size(); ++i)
        {
            bool is_x = (x_s[i] > x_s[i - 1] && x_s[i] > x_s[i + 1]);
            bool is_y = (y_s[i] > y_s[i - 1] && y_s[i] > y_s[i + 1]);
            bool is_z = (z_s[i] > z_s[i - 1] && z_s[i] > z_s[i + 1]);

            if (is_x || is_y || is_z)
            {
                fout << a_cur << ","
                     << (is_x ? std::to_string(x_s[i]) : "") << ","
                     << (is_y ? std::to_string(y_s[i]) : "") << ","
                     << (is_z ? std::to_string(z_s[i]) : "") << "\n";
                ++rows;
            }
        }

        a_cur += a_step;
    }

    fout.close();
    return rows;
}
