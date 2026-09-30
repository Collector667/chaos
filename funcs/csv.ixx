module;

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <type_traits>
#include "raylib.h"

#ifndef PROJECT_ROOT_DIR
#define PROJECT_ROOT_DIR "."
#endif

export module csv;

export template <typename StateType>
void writeTrajectoryCSV(const std::vector<StateType>& trajectory, double dt, const std::string& filename, int saveEvery = 1)
{
    std::filesystem::path dir = std::filesystem::path(PROJECT_ROOT_DIR) / "csv";
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }

    std::filesystem::path fullPath = dir / filename;

    std::ofstream file(fullPath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) return;

    file << "time,x,y,z\n";

    for (size_t i = 0; i < trajectory.size(); ++i) {
        if (i % saveEvery == 0) {
            double time = i * dt;
            if constexpr (std::is_same_v<StateType, Vector3>) {
                file << time << "," << trajectory[i].x << "," << trajectory[i].y << "," << trajectory[i].z << "\n";
            } else {
                file << time << "," << trajectory[i][0] << "," << trajectory[i][1] << "," << trajectory[i][2] << "\n";
            }
        }
    }
}