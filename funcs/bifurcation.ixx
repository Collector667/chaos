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
#include <unordered_set>

#include <Eigen/Dense>

export module bifurcation;

import system;

// --- Вспомогательные средства (не экспортируются: видны только внутри модуля) ---

// Признак ухода траектории в бесконечность.
bool isDiverged(const State& st)
{
    return !std::isfinite(st[0]) || !std::isfinite(st[1]) || !std::isfinite(st[2]) ||
           std::fabs(st[0]) > 1e6 || std::fabs(st[1]) > 1e6 || std::fabs(st[2]) > 1e6;
}

// Результат обсчёта одного узла сетки.
struct NodeResult
{
    std::size_t nuniq_x = 0, nuniq_y = 0, nuniq_z = 0;
    bool diverged = false;
};

// Обсчёт одного узла сетки: переходный процесс + подсчёт УНИКАЛЬНЫХ локальных
// максимумов x, y, z с точностью delta (значения округляются до сетки delta,
// считаются занятые корзины). Число шагов передаётся готовым: для сетки по dt
// оно зависит от текущего dt.
NodeResult analyzeNode(const auto& integrator, const auto& sys, double dt,
                       std::size_t steps_t, std::size_t steps_s, double delta)
{
    NodeResult r;

    // 1) Переходный процесс
    State s_state{0.1, 0.0, 0.0};
    for (std::size_t i = 0; i < steps_t; ++i)
    {
        s_state = integrator(s_state, dt, sys);
        if (isDiverged(s_state)) { r.diverged = true; return r; }
    }

    // 2) Установившийся участок: собираем УНИКАЛЬНЫЕ локальные максимумы
    std::unordered_set<long long> ux, uy, uz;
    double px = s_state[0], py = s_state[1], pz = s_state[2]; // prev
    double cx = px, cy = py, cz = pz;                         // cur

    for (std::size_t i = 0; i < steps_s; ++i)
    {
        s_state = integrator(s_state, dt, sys);
        if (isDiverged(s_state)) { r.diverged = true; return r; }

        double nxx = s_state[0], nyx = s_state[1], nzx = s_state[2]; // nxt

        if (cx > px && cx > nxx) ux.insert(std::llround(cx / delta));
        if (cy > py && cy > nyx) uy.insert(std::llround(cy / delta));
        if (cz > pz && cz > nzx) uz.insert(std::llround(cz / delta));

        px = cx; cx = nxx;
        py = cy; cy = nyx;
        pz = cz; cz = nzx;
    }

    r.nuniq_x = ux.size();
    r.nuniq_y = uy.size();
    r.nuniq_z = uz.size();
    return r;
}

// Строит бифуркационную диаграмму по параметру a = coefs[0],
// ищет локальные максимумы x,y,z на установившемся участке,
// записывает в CSV (p1,xmax,ymax,zmax).
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

    fout << "p1,xmax,ymax,zmax\n";
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

        // Система с текущими коэффициентами
        const CaseISystem sys_a{coefs};

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
        // Пишем через поток (а не std::to_string), чтобы работали
        // setprecision(9) и fixed, заданные выше.
        for (size_t i = 1; i + 1 < x_s.size(); ++i)
        {
            bool is_x = (x_s[i] > x_s[i - 1] && x_s[i] > x_s[i + 1]);
            bool is_y = (y_s[i] > y_s[i - 1] && y_s[i] > y_s[i + 1]);
            bool is_z = (z_s[i] > z_s[i - 1] && z_s[i] > z_s[i + 1]);

            if (is_x || is_y || is_z)
            {
                fout << a_cur << ",";
                if (is_x) fout << x_s[i];
                fout << ",";
                if (is_y) fout << y_s[i];
                fout << ",";
                if (is_z) fout << z_s[i];
                fout << "\n";
                ++rows;
            }
        }

        a_cur += a_step;
    }

    fout.close();
    return rows;
}

// Строит бифуркационную диаграмму по двум параметрам системы:
// p1 = coefs[idx1] и p2 = coefs[idx2]. Для каждого узла сетки считает
// число УНИКАЛЬНЫХ локальных максимумов x, y, z с точностью delta.
// Флаг diverged=1 ставится, если траектория ушла в бесконечность (NaN/Inf
// или |значение| > 1e6). diverged=0 при nuniq=0 означает равновесие/период-1.
// CSV: p1,p2,nuniq_x,nuniq_y,nuniq_z,diverged. Возвращает кол-во строк или -1.
export int buildBifurcation2D(const std::array<double, 6>& coefs_base,
                              size_t idx1, double p1_min, double p1_max, size_t N1,
                              size_t idx2, double p2_min, double p2_max, size_t N2,
                              double dt, double T_trans, double T_steady,
                              double delta,
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

    fout << "p1,p2,nuniq_x,nuniq_y,nuniq_z,diverged\n";
    fout << std::setprecision(9) << std::fixed;

    // Копия коэффициентов
    std::array<double, 6> coefs = coefs_base;

    int rows = 0;
    double p1_step = (p1_max - p1_min) / (double)(N1 - 1);
    double p2_step = (p2_max - p2_min) / (double)(N2 - 1);

    // dt фиксирован — число шагов одинаково для всех узлов
    const size_t steps_t = (size_t)std::ceil(T_trans / dt);
    const size_t steps_s = (size_t)std::ceil(T_steady / dt);

    double p1_cur = p1_min;

    for (size_t i1 = 0; i1 < N1; ++i1)
    {
        coefs[idx1] = p1_cur;
        double p2_cur = p2_min;

        for (size_t i2 = 0; i2 < N2; ++i2)
        {
            coefs[idx2] = p2_cur;

            // Система с текущими коэффициентами
            const CaseISystem sys_a{coefs};

            const NodeResult r = analyzeNode(integrator, sys_a, dt, steps_t, steps_s, delta);

            fout << p1_cur << "," << p2_cur << ","
                 << r.nuniq_x << "," << r.nuniq_y << "," << r.nuniq_z << ","
                 << (r.diverged ? 1 : 0) << "\n";
            ++rows;

            p2_cur += p2_step;
        }

        // Прогресс
        std::cerr << "  p1=" << p1_cur << " (" << (i1 + 1) << "/" << N1 << ") rows=" << rows << "\n";

        p1_cur += p1_step;
    }

    fout.close();
    return rows;
}

// Строит бифуркационную диаграмму по параметру a = coefs[0] и шагу интегрирования dt.
// Для каждого узла сетки считает число УНИКАЛЬНЫХ локальных максимумов x, y, z
// с точностью delta. T_trans и T_steady задаются в единицах времени (шагов = T/dt),
// поэтому при изменении dt меняется и число шагов.
// CSV: p1,p2,nuniq_x,nuniq_y,nuniq_z,diverged (p1 = a, p2 = dt).
// Возвращает кол-во строк или -1.
export int buildBifurcationA_dt(const std::array<double, 6>& coefs_base,
                                double a_min, double a_max, size_t Na,
                                double dt_min, double dt_max, size_t Ndt,
                                double T_trans, double T_steady,
                                double delta,
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

    fout << "p1,p2,nuniq_x,nuniq_y,nuniq_z,diverged\n";
    fout << std::setprecision(9) << std::fixed;

    // Копия коэффициентов
    std::array<double, 6> coefs = coefs_base;

    int rows = 0;
    double a_step = (a_max - a_min) / (double)(Na - 1);
    double dt_step = (dt_max - dt_min) / (double)(Ndt - 1);
    double a_cur = a_min;

    for (size_t ia = 0; ia < Na; ++ia)
    {
        coefs[0] = a_cur;
        double dt = dt_min;

        for (size_t idt = 0; idt < Ndt; ++idt)
        {
            // Система с текущими коэффициентами
            const CaseISystem sys_a{coefs};

            // dt меняется — число шагов пересчитываем для каждого узла
            const size_t steps_t = (size_t)std::ceil(T_trans / dt);
            const size_t steps_s = (size_t)std::ceil(T_steady / dt);

            const NodeResult r = analyzeNode(integrator, sys_a, dt, steps_t, steps_s, delta);

            fout << a_cur << "," << dt << ","
                 << r.nuniq_x << "," << r.nuniq_y << "," << r.nuniq_z << ","
                 << (r.diverged ? 1 : 0) << "\n";
            ++rows;

            dt += dt_step;
        }

        // Прогресс
        std::cerr << "  a=" << a_cur << " (" << (ia + 1) << "/" << Na << ") rows=" << rows << "\n";

        a_cur += a_step;
    }

    fout.close();
    return rows;
}
