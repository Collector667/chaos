#include <iostream>
#include <vector>
#include "raylib.h"
#include "rcamera.h"
#include <Eigen/Dense>



import CDMethod;
import EulerCramerMethod;
import EulerMethod;
import MidPointMethod;
import RungeKuttaMethods;
import csv;
import chaosRisuyka;




//template <typename SystemFunction>




//int main()
//{
//
//     Params p;
//
//     auto dx = [&p](double x, double y, double z) { return p.a * y * p.scale; };
//     auto dy = [&p](double x, double y, double z) { return (p.b * x + p.c * z) * p.scale; };
//     auto dz = [&p](double x, double y, double z) { return (p.d * x + p.e * y * y + p.f * z) * p.scale; };
//
//     double x0 = 0.0, y0 = 0.1, z0 = 0.0;
//     const double h = 1e-6;
//     const double timeEnd = 20.0;
//     const int saveEvery = 1000;
//
//     solve(eulerStep, dx, dy, dz, x0, y0, z0, h, timeEnd, saveEvery, "csv/euler.csv");
//     // solve(midPointStep, dx, dy, dz, x0, y0, z0, h, timeEnd, saveEvery, "csv/midPoint.csv");
//     solve(eulerCromerStep, dx, dy, dz, x0, y0, z0, h, timeEnd, saveEvery, "csv/eulerCromer.csv");
//}


struct Params {
     double a     = -0.2;
     double b     = 1.0;
     double c     = 1.0;
     double d     = 1.0;
     double e     = 1.0;
     double f     = -1.0;
     double scale = 1e0;

};

using State = Eigen::Vector3d;


Params p;

// =====================================================================
// ПРИМЕР ИСПОЛЬЗОВАНИЯ В MAIN (Генерация двух систем)
// =====================================================================
State lorenz(const State& s) {

    State ds;
    ds[0] = 10.0 * (s[1] - s[0]);
    ds[1] = s[0] * (28.0 - s[2]) - s[1];
    ds[2] = s[0] * s[1] - (8.0/3.0) * s[2];
    return ds;
}

inline auto CaseI = [](const State& s) -> State {

    return State {
        p.a * s[1],
        p.b * s[0] + p.c * s[2],
        p.d * s[0] + p.e * s[1] * s[1] + p.f * s[2]
    } * p.scale;
};

inline auto CaseI_ = [](const State& s) -> State {

    return State {
        p.a * s[1],
        p.b * s[0] + p.c * s[2],
        (p.d * s[0] + p.e * s[1] * s[1] - p.f * s[2])*0.5
    } * p.scale;
};




int main() {

    AppConfig CFG;
//    CFG.offsetZ = -25.0f; // Центруем аттрактор по высоте

    int totalSteps = 200000;
    double dt = 0.01;


    // Векторы для хранения всех наших графиков
    std::vector<std::vector<Vector3>> allTrajectories;
    std::vector<std::vector<Color>> allColors;

    // --- ГРАФИК 1 (Красный градиент, старт из 1.0, 1.0, 1.0) ---
    State state1(0.1, 0.0, 0.0);
    std::vector<Vector3> traj1;
    std::vector<Color> col1;

    // --- ГРАФИК 2 (Синий градиент, старт сдвинут всего на 0.001) ---
    State state2(0.1, 0.0, 0.0);
    std::vector<Vector3> traj2;
    std::vector<Color> col2;

    State state3(0.1, 0.0, 0.0);
    std::vector<Vector3> traj3;
    std::vector<Color> col3;

    State state4(0.1, 0.0, 0.0);
    std::vector<Vector3> traj4;
    std::vector<Color> col4;

    State state5(0.1, 0.0, 0.0);
    std::vector<Vector3> traj5;
    std::vector<Color> col5;
    for (int i = 0; i < totalSteps; ++i) {
        // Точки первой системы
        traj1.push_back(Vector3{(float)state1[0], (float)(state1[2] ), (float)state1[1]});
        float t1 = (float)i / totalSteps;
        col1.push_back(Color{ 255, 0, 0, 255 }); // От черного к красному
        state1 = euler(state1, dt, CaseI);


        // Точки второй системы
       traj2.push_back(Vector3{(float)state2[0], (float)(state2[2] ), (float)state2[1]});
       float t2 = (float)i / totalSteps;
       col2.push_back(Color{ 0, 200, 255, 255 }); // От черного к голубому
       state2 = midPoint(state2, dt, CaseI);

       // Точки второй системы
       traj3.push_back(Vector3{(float)state3[0], (float)(state3[2] + CFG.offsetZ), (float)state3[1]});
       float t3 = (float)i / totalSteps;
       col3.push_back(Color{ 0, 255, 0, 255 }); // От черного к голубому
       state3 = eulerCramer(state3, dt, CaseI);

       // Точки второй системы
       traj4.push_back(Vector3{(float)state4[0], (float)(state4[2] + CFG.offsetZ), (float)state4[1]});
       float t4 = (float)i / totalSteps;
       col4.push_back(Color{ 0, 120, 0, 255 }); // От черного к голубому
       state4 = rk4(state4, dt, CaseI);

       traj5.push_back(Vector3{(float)state5[0], (float)(state5[2] + CFG.offsetZ), (float)state5[1]});
       float t5 = (float)i / totalSteps;
       col5.push_back(Color{ 0, 120, 0, 255 }); // От черного к голубому
       state5 = CD(state5, dt, CaseI, CaseI_);

    }
    // Запихиваем оба графика в главный вектор
//
    allTrajectories.push_back(traj1);
    allTrajectories.push_back(traj2);
    allTrajectories.push_back(traj3);
    allTrajectories.push_back(traj4);
    allTrajectories.push_back(traj5);
////
////
////
    allColors.push_back(col1);
    allColors.push_back(col2);
    allColors.push_back(col3);
    allColors.push_back(col4);
    allColors.push_back(col5);


    writeTrajectoryCSV(traj1, dt, "Euler", 1);
    writeTrajectoryCSV(traj2, dt, "midPoint", 1);
    writeTrajectoryCSV(traj3, dt, "EulerCramer", 1);
    writeTrajectoryCSV(traj4, dt, "rk4", 1);
    writeTrajectoryCSV(traj5, dt, "CD", 1);



    // Вызываем визуализатор для ВСЕХ графиков сразу!
    // VisualizeSystem(allTrajectories, allColors, CFG);

    return 0;
}

