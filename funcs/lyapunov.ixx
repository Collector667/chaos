module;

#include <fstream>
#include <filesystem>
#include <iostream>
#include <array>
#include <cmath>
#include <string>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <vector>
#include <atomic>
#include <thread>

#include <Eigen/Dense>

export module lyapunov;

import system;

// --- Вспомогательные средства (не экспортируются: видны только внутри модуля) ---

// Сколько потоков использовать. LYAP_THREADS задаёт число вручную
// (нужно для проверки: 1 поток должен дать побайтно тот же CSV),
// иначе берётся hardware_concurrency(). Больше узлов, чем потоков, не нужно.
unsigned lyapThreads(std::size_t nodes)
{
    unsigned nt = 0;

    if (const char* env = std::getenv("LYAP_THREADS"))
    {
        const unsigned long v = std::strtoul(env, nullptr, 10);
        if (v > 0) nt = (unsigned)v;
    }

    if (nt == 0) nt = std::thread::hardware_concurrency();
    if (nt == 0) nt = 1;
    if ((std::size_t)nt > nodes) nt = (unsigned)nodes;
    return nt;
}

// Параллельный обход независимых индексов 0..n-1.
// Динамическая раздача через атомарный счётчик: стоимость узлов разная
// (в сетке по dt она меняется в 100 раз), поэтому статическое деление
// диапазона дало бы перекос. Запись результатов — забота вызывающего кода:
// каждый индекс должен писаться только своим потоком.
template <class F>
void parallelFor(std::size_t n, F&& fn)
{
    const unsigned nt = lyapThreads(n);

    if (nt <= 1)
    {
        for (std::size_t i = 0; i < n; ++i) fn(i);
        return;
    }

    std::atomic<std::size_t> next{0};

    auto worker = [&]
    {
        for (;;)
        {
            const std::size_t i = next.fetch_add(1, std::memory_order_relaxed);
            if (i >= n) return;
            fn(i);
        }
    };

    std::vector<std::thread> pool;
    pool.reserve(nt - 1);
    for (unsigned t = 1; t < nt; ++t) pool.emplace_back(worker);
    worker();                       // главный поток тоже считает
    for (auto& th : pool) th.join();
}

// Результат оценки показателя в одном узле.
export struct LyapResult
{
    double lambda = std::numeric_limits<double>::quiet_NaN();
    bool diverged = false;
};

// Старший показатель Ляпунова методом Бенеттина (две траектории).
//
//   1) транзиент только по опорной траектории U;
//   2) возмущённая траектория V = U + (d0, 0, 0), |V - U| = d0;
//   3) на каждом шаге: d_new = |V - U|, S += ln(d_new / d0),
//      V <- U + d0 * (V - U) / d_new  (перенормировка);
//   4) lambda = S / (steps_m * dt).
//
// Перенормировка каждый шаг (k = 1), поэтому steps_m — это ровно N
// в формуле, без множителя на число шагов между перенормировками.
LyapResult lyapunovNode(const auto& integrator, const CaseISystem& sys,
                        double dt, std::size_t steps_t, std::size_t steps_m,
                        double d0)
{
    LyapResult res;

    // 1) Транзиент: только опорная траектория, выводим её на аттрактор
    State u{0.1, 0.0, 0.0};
    for (std::size_t i = 0; i < steps_t; ++i)
    {
        u = integrator(u, dt, sys);
        if (isDiverged(u)) { res.diverged = true; return res; }
    }

    // 2) Возмущённая траектория: сдвиг на d0 вдоль x
    State v = u;
    v[0] += d0;

    // 3) Измерение: перенормировка на каждом шаге
    double sum = 0.0;
    for (std::size_t i = 0; i < steps_m; ++i)
    {
        u = integrator(u, dt, sys);
        v = integrator(v, dt, sys);

        if (isDiverged(u) || isDiverged(v)) { res.diverged = true; return res; }

        const double d_new = (v - u).norm();
        if (!(d_new > 0.0) || !std::isfinite(d_new)) { res.diverged = true; return res; }

        sum += std::log(d_new / d0);
        v = u + (d0 / d_new) * (v - u);
    }

    // 4) Усреднение
    res.lambda = sum / ((double)steps_m * dt);
    return res;
}

// Строит lambda_max(a) для a = coefs[0].
// CSV: p1,lambda,diverged. Возвращает число строк или -1.
export int buildLyapunovA(const std::array<double, 6>& coefs_base,
                          double a_min, double a_max, std::size_t Na,
                          double dt, double T_trans, double T_meas,
                          double d0,
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

    fout << "p1,lambda,diverged\n";
    fout << std::setprecision(9) << std::fixed;

    // Копия коэффициентов
    // (в параллельном цикле у каждого узла она своя)

    const std::size_t steps_t = (std::size_t)std::ceil(T_trans / dt);
    const std::size_t steps_m = (std::size_t)std::ceil(T_meas / dt);

    const double a_step = (Na > 1) ? (a_max - a_min) / (double)(Na - 1) : 0.0;

    // Узлы независимы: считаем параллельно, каждый пишет в свой элемент.
    std::vector<double> a_val(Na);
    std::vector<LyapResult> res(Na);

    parallelFor(Na, [&](std::size_t i)
    {
        a_val[i] = a_min + (double)i * a_step;

        std::array<double, 6> coefs = coefs_base;
        coefs[0] = a_val[i];

        // Система с текущими коэффициентами
        const CaseISystem sys_a{coefs};

        res[i] = lyapunovNode(integrator, sys_a, dt, steps_t, steps_m, d0);
    });

    // Запись строго по индексам — порядок строк детерминирован.
    for (std::size_t i = 0; i < Na; ++i)
        fout << a_val[i] << "," << res[i].lambda << ","
             << (res[i].diverged ? 1 : 0) << "\n";

    fout.close();
    return (int)Na;
}

// Строит lambda_max по двум параметрам p1 = coefs[idx1], p2 = coefs[idx2].
// Сетка N1 x N2, шаг интегрирования dt фиксирован.
// CSV: p1,p2,lambda,diverged. Возвращает число строк или -1.
export int buildLyapunov2D(const std::array<double, 6>& coefs_base,
                           std::size_t idx1, double p1_min, double p1_max, std::size_t N1,
                           std::size_t idx2, double p2_min, double p2_max, std::size_t N2,
                           double dt, double T_trans, double T_meas, double d0,
                           const std::string& filename,
                           auto integrator)
{
    std::filesystem::path csv_dir = std::filesystem::current_path() / "csv";
    if (!std::filesystem::exists(csv_dir))
        std::filesystem::create_directories(csv_dir);

    std::ofstream fout(csv_dir / filename);
    if (!fout.is_open())
    {
        std::cerr << "Не удалось открыть " << (csv_dir / filename) << "\n";
        return -1;
    }

    fout << "p1,p2,lambda,diverged\n";
    fout << std::setprecision(9) << std::fixed;

    const std::size_t steps_t = (std::size_t)std::ceil(T_trans / dt);
    const std::size_t steps_m = (std::size_t)std::ceil(T_meas / dt);

    const double p1_step = (N1 > 1) ? (p1_max - p1_min) / (double)(N1 - 1) : 0.0;
    const double p2_step = (N2 > 1) ? (p2_max - p2_min) / (double)(N2 - 1) : 0.0;

    const std::size_t total = N1 * N2;

    std::vector<double> v1(total), v2(total);
    std::vector<LyapResult> res(total);

    // p1 — внешний индекс, p2 — внутренний (порядок строк как у lab2).
    parallelFor(total, [&](std::size_t k)
    {
        const std::size_t i = k / N2;
        const std::size_t j = k % N2;

        v1[k] = p1_min + (double)i * p1_step;
        v2[k] = p2_min + (double)j * p2_step;

        std::array<double, 6> coefs = coefs_base;
        coefs[idx1] = v1[k];
        coefs[idx2] = v2[k];

        res[k] = lyapunovNode(integrator, CaseISystem{coefs}, dt, steps_t, steps_m, d0);
    });

    for (std::size_t k = 0; k < total; ++k)
        fout << v1[k] << "," << v2[k] << "," << res[k].lambda << ","
             << (res[k].diverged ? 1 : 0) << "\n";

    fout.close();
    return (int)total;
}

// Строит lambda_max(a, dt): p1 = a = coefs[0], p2 = dt.
// В отличие от buildLyapunov2D, шаг интегрирования меняется по сетке,
// поэтому число шагов транзиента/измерения пересчитывается в каждом узле
// из T_trans и T_meas (они заданы в единицах времени).
// CSV: p1,p2,lambda,diverged. Возвращает число строк или -1.
export int buildLyapunovA_dt(const std::array<double, 6>& coefs_base,
                             double a_min, double a_max, std::size_t Na,
                             double dt_min, double dt_max, std::size_t Ndt,
                             double T_trans, double T_meas, double d0,
                             const std::string& filename,
                             auto integrator)
{
    std::filesystem::path csv_dir = std::filesystem::current_path() / "csv";
    if (!std::filesystem::exists(csv_dir))
        std::filesystem::create_directories(csv_dir);

    std::ofstream fout(csv_dir / filename);
    if (!fout.is_open())
    {
        std::cerr << "Не удалось открыть " << (csv_dir / filename) << "\n";
        return -1;
    }

    fout << "p1,p2,lambda,diverged\n";
    fout << std::setprecision(9) << std::fixed;

    const double a_step  = (Na  > 1) ? (a_max  - a_min)  / (double)(Na  - 1) : 0.0;
    const double dt_step = (Ndt > 1) ? (dt_max - dt_min) / (double)(Ndt - 1) : 0.0;

    const std::size_t total = Na * Ndt;

    std::vector<double> a_val(total), dt_val(total);
    std::vector<LyapResult> res(total);

    parallelFor(total, [&](std::size_t k)
    {
        const std::size_t i = k / Ndt;
        const std::size_t j = k % Ndt;

        a_val[k]  = a_min  + (double)i * a_step;
        dt_val[k] = dt_min + (double)j * dt_step;

        std::array<double, 6> coefs = coefs_base;
        coefs[0] = a_val[k];

        const double h = dt_val[k];
        const std::size_t st = (std::size_t)std::ceil(T_trans / h);
        const std::size_t sm = (std::size_t)std::ceil(T_meas  / h);

        res[k] = lyapunovNode(integrator, CaseISystem{coefs}, h, st, sm, d0);
    });

    for (std::size_t k = 0; k < total; ++k)
        fout << a_val[k] << "," << dt_val[k] << "," << res[k].lambda << ","
             << (res[k].diverged ? 1 : 0) << "\n";

    fout.close();
    return (int)total;
}
