#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raylib-cpp.hpp"
#include "Graph.h"
#include "Map.h"
#include "Population.h"
#include "SimulationTime.h"
#include "DiseaseParameters.h"

int main() {
    // Initialization
    //--------------------------------------------------------------------------------------
    // Window & scene
    typedef enum ApplicationScreen {MENU, INITIALIZATION, SIMULATION};
    ApplicationScreen currentscreen = MENU;

    const int screenWidth = 1800;
    const int screenHeight = 900;
    raylib::Window window(screenWidth, screenHeight, "Pandemic Simulator");
    SetTargetFPS(60);
    

    // Graph and helper variables
    int filler{}, frameCounter{};
    Graph graph(100, 50, 705, 450);

    // Camera
    raylib::Vector2 screenCenter = { screenWidth / 2, screenHeight / 2 };
    raylib::Camera2D camera;
    camera.offset = screenCenter;
    camera.target = { -1100, 0 };
    camera.rotation = 0.0f;
    camera.zoom = 0.4f;

    // Disease parameters to be changed
    DiseaseParameters diseaseParameters;
    diseaseParameters.infectionProbabilityPerHour = 0.05f;
    diseaseParameters.deathProbabilityPerHour = 0.005f;
    diseaseParameters.hoursToGetImmune = 24.0f;
    diseaseParameters.hoursToGetSymptoms = 12.0f;
    float simulationHourTime = 1.0f;
    int populationSize = 500;
    int residentsLimit = 4;

    // Initialize and generate the map
    Map map(populationSize, residentsLimit);
    map.GenerateMap();
    map.GenerateBuildings();

    // Constant disease parameters
    diseaseParameters.infectionRadius = 20.0f;
    diseaseParameters.probabilityOfGoingToHospitalPerHour = 0.01f;
    diseaseParameters.deathProbabilityPerHourInHospital = 0.002f;

    // Initialize simulation time object
    SimulationTime simulationTime(simulationHourTime);

    // Initialize the population
    Population population(populationSize, &map, diseaseParameters, residentsLimit);
 
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!window.ShouldClose()) {   // Detect window close button or ESC key
        // Update
        switch (currentscreen) {
            case MENU: {
                if (IsKeyPressed(KEY_ENTER)) { currentscreen = INITIALIZATION; }
                } break;
            case INITIALIZATION: {
                simulationTime.ChangeHourLength(simulationHourTime);
                population.ChangePopulationParameters(&diseaseParameters);

                population.UpdateSimulationSpeed(simulationTime.GetHourLength());

                currentscreen = SIMULATION;
            } break;
            case SIMULATION: {
                // ----- Graph handling -----
                // phase 1: initial fill-in
                if (filler < graph.getWidth() - graph.getAxisWidth()) {
                    graph.updateGraphStart(&filler, simulationTime.GetHourLength(), &population);
                }
                else {
                    graph.updateGraph(&frameCounter, simulationTime.GetHourLength(), &population);
                }
                // reset frameCounter for counting hours in TimeUnits
                if (frameCounter == trunc(ceil(simulationTime.GetHourLength() * 60.f)) * 24) {
                    frameCounter = 0;
                }
                frameCounter++;

                // ----- Camera handling -----
                float wheel = GetMouseWheelMove();

                // Camera movement (when dragged with left mouse button)
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                {
                    raylib::Vector2 mouseMovement(GetMouseDelta());
                    mouseMovement = Vector2Scale(mouseMovement, -1.0f / camera.zoom);
                    camera.target = Vector2Add(camera.target, mouseMovement);
                }

                // Camera zooming (when mouse scroll used)
                if (wheel != 0)
                {
                    raylib::Vector2 mouseWorldPosition = GetScreenToWorld2D(GetMousePosition(), camera);
                    camera.offset = GetMousePosition();
                    camera.target = mouseWorldPosition;

                    // Zoom increment
                    float scale = 0.1f * wheel;
                    camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.3f, 0.9f);
                }

                // ----- Simulation handling -----
                float deltaTime = GetFrameTime();

                // Update global simulation time and population's current buildings based on schedules
                simulationTime.AdvanceTime(deltaTime);

                if (simulationTime.HasHourChanged()) {
                    population.UpdatePopulationOnHour(simulationTime.GetHour());
                    std::cout << "Current hour: " << simulationTime.GetHour() << std::endl;
                }

                population.UpdatePopulationOnFrame(deltaTime);
            } break;
        }

        // Draw
        //----------------------------------------------------------------------------------
        window.BeginDrawing();
        {
            switch (currentscreen) {
                case MENU: {
                    window.ClearBackground(raylib::Color::LightGray());
                    raylib::Color::Black().DrawText("Epidemic Simulator", 400, 50, 90);
                    raylib::Color::Black().DrawText("Choose starting parameters: ", 400, 150, 60);
                    raylib::Color::Black().DrawText("Press Enter to begin... ", 400, 800, 40);

                    GuiSliderBar(raylib::Rectangle(650, 250, 400, 100), "Hour Duration", TextFormat("%.2f (seconds)", simulationHourTime), &simulationHourTime, 0.75, 2);
                    GuiSliderBar(raylib::Rectangle(650, 375, 400, 100), "Infection Probability", TextFormat("%.1f %% (per hour)", diseaseParameters.infectionProbabilityPerHour * 100),
                    &diseaseParameters.infectionProbabilityPerHour, 0.005, 0.05);
                    GuiSliderBar(raylib::Rectangle(650, 500, 400, 100), "Death Probability", TextFormat("%.1f %% (per hour)", diseaseParameters.deathProbabilityPerHour * 100),
                    &diseaseParameters.deathProbabilityPerHour, 0.001, 0.01);
                    GuiSliderBar(raylib::Rectangle(650, 625, 400, 100), "Disease duration", TextFormat("%.1f (hours)", diseaseParameters.hoursToGetImmune),
                    &diseaseParameters.hoursToGetImmune, 12, 72);

                } break;
                case SIMULATION: {
                    // background
                    window.ClearBackground(raylib::Color::RayWhite());

                    // map
                    BeginMode2D(camera); 
                    {
                        map.DrawMap();
                        map.DrawBuildings();
                        population.DrawPopulation();
                    }
                    EndMode2D();

                    // stats section
                    raylib::Color(0, 0, 0, 150).DrawRectangle(200, screenHeight - 350, 450, 320);
                    raylib::Color::RayWhite().DrawText("Day: " + std::to_string(simulationTime.GetDay()), 225, screenHeight - 330, 40);
                    raylib::Color::RayWhite().DrawText("Hour: " + std::to_string(simulationTime.GetHour()), 225, screenHeight - 280, 40);
                    raylib::Color::Green().DrawText("Healthy: " + std::to_string(population.GetHealthyCount()), 225, screenHeight - 230, 40);
                    raylib::Color::Red().DrawText("Infected: " + std::to_string(population.GetInfectedCount()), 225, screenHeight - 180, 40);
                    raylib::Color::SkyBlue().DrawText("Immune: " + std::to_string(population.GetImmuneCount()), 225, screenHeight - 130, 40);
                    raylib::Color::Black().DrawText("Dead: " + std::to_string(population.GetDeadCount()), 225, screenHeight - 80, 40);

                    // window & graph
                    window.DrawFPS();
                    graph.drawGraph();
                    

                } break;
            }
        }
        window.EndDrawing();
        //----------------------------------------------------------------------------------
    }
    return 0;
}