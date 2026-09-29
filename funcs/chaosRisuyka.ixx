//
// Created by Муслихиддин Махмудов on 29.09.2026.
//
module;
#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include "raylib.h"
#include "rcamera.h"
export module chaosRisuyka;



using State = Eigen::Vector3d;

// =====================================================================
// КОНФИГУРАЦИЯ ВИЗУАЛИЗАТОРА
// =====================================================================
export struct AppConfig {
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float offsetZ = 0.0f;

    int gridSize = 5;
    int gridStep = 1;
    float camDist = 6.0f;
    float sphere = 0.01f;

    int animationSpeed = 100;
    int maxVisiblePoints = 2000;
};

// =====================================================================
// УНИВЕРСАЛЬНАЯ ФУНКЦИЯ ДЛЯ МНОЖЕСТВА ГРАФИКОВ
// Принимает вектор векторов (trajectories) и такой же вектор цветов (colors)
// =====================================================================
export void VisualizeSystem(
    const std::vector<std::vector<Vector3>>& trajectories,
    const std::vector<std::vector<Color>>& colors,
    const AppConfig& CFG
) {
    if (trajectories.empty() || trajectories.size() != colors.size()) {
        std::cerr << "Ошибка: Массивы графиков и цветов пусты или не совпадают по размеру!\n";
        return;
    }

    InitWindow(1024, 768, "Multi-Graph 3D Visualizer");

    int codepoints[512] = { 0 };
    int codepointCount = 0;
    for (int i = 32; i < 127; i++) codepoints[codepointCount++] = i;
    for (int i = 1024; i < 1105; i++) codepoints[codepointCount++] = i;
    codepoints[codepointCount++] = 1105;
    codepoints[codepointCount++] = 1025;
    Font font = LoadFontEx("/Library/Fonts/Arial.ttf", 20, codepoints, codepointCount);

    Camera3D camera = { 0 };
    camera.position = Vector3{ CFG.camDist, CFG.camDist * 0.7f, CFG.camDist };
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    int currentPointsToDraw = 1;
    bool isPaused = false;

    // Ищем самую длинную траекторию, чтобы знать, когда останавливать анимацию
    int maxTotalPoints = 0;
    for (const auto& traj : trajectories) {
        if (traj.size() > maxTotalPoints) maxTotalPoints = traj.size();
    }

    while (!WindowShouldClose()) {

        // --- 1. ПАУЗА И ПЕРЕМОТКА ---
        if (IsKeyPressed(KEY_SPACE)) isPaused = !isPaused;

        if (isPaused) {
            if (IsKeyDown(KEY_RIGHT)) {
                currentPointsToDraw += CFG.animationSpeed;
                if (currentPointsToDraw > maxTotalPoints) currentPointsToDraw = maxTotalPoints;
            }
            if (IsKeyDown(KEY_LEFT)) {
                currentPointsToDraw -= CFG.animationSpeed;
                if (currentPointsToDraw < 1) currentPointsToDraw = 1;
            }
        } else {
            if (currentPointsToDraw < maxTotalPoints) {
                currentPointsToDraw += CFG.animationSpeed;
                if (currentPointsToDraw > maxTotalPoints) currentPointsToDraw = maxTotalPoints;
            }
        }

        // --- 2. КАМЕРА ---
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mouseDelta = GetMouseDelta();
            CameraYaw(&camera, -mouseDelta.x * 0.005f, false);
            CameraPitch(&camera, -mouseDelta.y * 0.005f, true, false, false);
        }

        float moveSpeed = CFG.gridSize * 0.0125f;
        if (IsKeyDown(KEY_W)) CameraMoveForward(&camera, moveSpeed, true);
        if (IsKeyDown(KEY_S)) CameraMoveForward(&camera, -moveSpeed, true);
        if (IsKeyDown(KEY_D)) CameraMoveRight(&camera, moveSpeed, true);
        if (IsKeyDown(KEY_A)) CameraMoveRight(&camera, -moveSpeed, true);
        if (IsKeyDown(KEY_E)) CameraMoveUp(&camera, moveSpeed);
        if (IsKeyDown(KEY_Q)) CameraMoveUp(&camera, -moveSpeed);

        // --- 3. РЕНДЕР ---
        BeginDrawing();
            ClearBackground(Color{ 18, 18, 24, 255 });

            BeginMode3D(camera);

                // 3D сетка
                Color gridColor = Color{ 70, 70, 85, 120 };
                for (int i = -CFG.gridSize; i <= CFG.gridSize; i += CFG.gridStep) {
                    DrawLine3D(Vector3{(float)i, 0, (float)-CFG.gridSize}, Vector3{(float)i, 0, (float)CFG.gridSize}, gridColor);
                    DrawLine3D(Vector3{(float)-CFG.gridSize, 0, (float)i}, Vector3{(float)CFG.gridSize, 0, (float)i}, gridColor);
                    DrawLine3D(Vector3{(float)i, (float)-CFG.gridSize, 0}, Vector3{(float)i, (float)CFG.gridSize, 0}, gridColor);
                    DrawLine3D(Vector3{(float)-CFG.gridSize, (float)i, 0}, Vector3{(float)CFG.gridSize, (float)i, 0}, gridColor);
                    DrawLine3D(Vector3{0, (float)i, (float)-CFG.gridSize}, Vector3{0, (float)i, (float)CFG.gridSize}, gridColor);
                    DrawLine3D(Vector3{0, (float)-CFG.gridSize, (float)i}, Vector3{0, (float)CFG.gridSize, (float)i}, gridColor);
                }

                DrawLine3D(Vector3{(float)-CFG.gridSize, 0, 0}, Vector3{(float)CFG.gridSize, 0, 0}, RED);
                DrawLine3D(Vector3{0, (float)-CFG.gridSize, 0}, Vector3{0, (float)CFG.gridSize, 0}, GREEN);
                DrawLine3D(Vector3{0, 0, (float)-CFG.gridSize}, Vector3{0, 0, (float)CFG.gridSize}, BLUE);

                float tickSize = CFG.gridSize * 0.0125f;
                for (int i = -CFG.gridSize; i <= CFG.gridSize; i += CFG.gridStep) {
                    if (i == 0) continue;
                    DrawLine3D(Vector3{(float)i, -tickSize, 0}, Vector3{(float)i, tickSize, 0}, RED);
                    DrawLine3D(Vector3{-tickSize, (float)i, 0}, Vector3{tickSize, (float)i, 0}, GREEN);
                    DrawLine3D(Vector3{0, -tickSize, (float)i}, Vector3{0, tickSize, (float)i}, BLUE);
                }

        // =========================================================
        // ОТРИСОВКА ВСЕХ ГРАФИКОВ
        // =========================================================
        for (size_t g = 0; g < trajectories.size(); ++g) {
            const auto& pts = trajectories[g];
            const auto& cls = colors[g];

            if (pts.empty()) continue;

            int drawLimit = (currentPointsToDraw > pts.size()) ? pts.size() : currentPointsToDraw;
            int startIndex = 1;
            if (drawLimit > CFG.maxVisiblePoints) {
                startIndex = drawLimit - CFG.maxVisiblePoints + 1;
            }

            for (int i = startIndex; i < drawLimit; ++i) {
                // Рисуем линию строго тем цветом, который передали в массиве
                DrawLine3D(pts[i - 1], pts[i], cls[i]);
            }

            if (drawLimit > 0) {
                // "Голову" графика тоже красим в цвет последней активной точки
                DrawSphere(pts[drawLimit - 1], CFG.sphere, cls[drawLimit - 1]);
            }
        }
        // =========================================================

            EndMode3D();

            // --- 4. 2D ИНТЕРФЕЙС ---
            float labelOffset = CFG.gridSize + (CFG.gridSize * 0.05f);
            Vector2 labelX = GetWorldToScreen(Vector3{labelOffset, 0, 0}, camera);
            Vector2 labelZ = GetWorldToScreen(Vector3{0, labelOffset, 0}, camera);
            Vector2 labelY = GetWorldToScreen(Vector3{0, 0, labelOffset}, camera);
            DrawText("X", labelX.x, labelX.y, 20, RED);
            DrawText("Z", labelZ.x, labelZ.y, 20, GREEN);
            DrawText("Y", labelY.x, labelY.y, 20, BLUE);

            for (int i = -CFG.gridSize; i <= CFG.gridSize; i += CFG.gridStep) {
                if (i == 0) continue;
                Vector2 posX = GetWorldToScreen(Vector3{(float)i, -tickSize*3, 0}, camera);
                DrawText(TextFormat("%d", (int)(i - CFG.offsetX)), posX.x - 10, posX.y, 10, PINK);
                Vector2 posY = GetWorldToScreen(Vector3{0, -tickSize*3, (float)i}, camera);
                DrawText(TextFormat("%d", (int)(i - CFG.offsetY)), posY.x - 10, posY.y, 10, SKYBLUE);
                Vector2 posZ = GetWorldToScreen(Vector3{-tickSize*3, (float)i, 0}, camera);
                DrawText(TextFormat("%d", (int)(i - CFG.offsetZ)), posZ.x - 10, posZ.y, 10, LIGHTGRAY);
            }

            if (isPaused) {
                DrawTextEx(font, "[Pause] left (t-) right (t+)", Vector2{20.0f, 15.0f}, 20.0f, 1.0f, YELLOW);
            } else {
                DrawTextEx(font, "Spacebar - Pause | mouse cam | WASD - Fly", Vector2{20.0f, 15.0f}, 18.0f, 1.0f, RAYWHITE);
            }

            DrawTextEx(font, TextFormat("Graph count: %lu  |  step: %d / %d", trajectories.size(), currentPointsToDraw, maxTotalPoints), Vector2{20.0f, 45.0f}, 18.0f, 1.0f, LIGHTGRAY);
            DrawFPS(20, 75);

        EndDrawing();
    }

    UnloadFont(font);
    CloseWindow();
}
