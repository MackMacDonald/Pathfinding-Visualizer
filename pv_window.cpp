#include "raylib.h"

#include "pv_gui.h"
#include "pv_algorithms.h"

#include <thread>
#include <atomic>

// Thread synchronization
static std::atomic<bool> done(false);

struct ControlState
{
    int pathfindingButtonIndex;
    int cellButtonIndex;

    int startCellIndex;
    int endCellIndex;

    Color cellDrawColor;
};

int main()
{
    // Initialization
    const int gridRows = 50;
    const int gridCols = 50;
    float screenWidth = 1280;
    float screenHeight = 720;

    ControlState state = {0, 0, 1250, 1299, RAYWHITE};

    std::thread thread;

    // ID for gui elements
    int id = 0;

    InitWindow(screenWidth, screenHeight, "Pathfinding Visualizer");
    SetWindowMinSize(screenWidth, screenHeight);
    SetWindowState(FLAG_VSYNC_HINT | FLAG_WINDOW_ALWAYS_RUN | FLAG_WINDOW_RESIZABLE);

    // Region for all of the buttons
    Rectangle controlRegion = {0, 0, screenWidth / 5, screenHeight};
    Rectangle gridRegion = {controlRegion.width, 0, screenWidth - controlRegion.width, screenHeight};
    gridRegion = CenterAndSizeGuiElement(gridRegion, 1, 0, squareFill);

    // Initialize button regions
    Rectangle buttonRegions[4];
    int buttonRegionArraySize = 4;

    for (int i = 0; i < buttonRegionArraySize; i++)
    {
        buttonRegions[i] = CenterAndSizeGuiElement(controlRegion, buttonRegionArraySize, i, verticalFill);
    }

    // Initialize pathfinding buttons
    enum PathfindingTypes
    {
        depthFirstSearch,
        breadthFirstSearch,
        dijkstra,
        aStar
    };
    const char *pathfindingButtonText[] = {"Depth First Search", "Breadth First Search", "Dijkstra's", "A*"};
    int pathfindingArraySize = 4;
    Button pathfindingButtons[pathfindingArraySize];

    for (int i = 0; i < pathfindingArraySize; i++)
    {
        pathfindingButtons[i] = {CenterAndSizeGuiElement(buttonRegions[0], pathfindingArraySize, i, verticalSpaced), pathfindingButtonText[i], buttonTypeText, LIGHTGRAY, id++};
    }

    // Initialize maze buttons
    enum MazeTypes
    {
        prim,
        dfs,
        sidewinder
    };
    const char *mazeButtonText[] = {"Randomized Prim's Maze", "Randomized DFS Maze", "Sidewinder Maze"};
    int mazeArraySize = 3;
    Button mazeButtons[mazeArraySize];

    for (int i = 0; i < mazeArraySize; i++)
    {
        mazeButtons[i] = {CenterAndSizeGuiElement(buttonRegions[1], mazeArraySize, i, verticalSpaced), mazeButtonText[i], buttonTypeText, LIGHTGRAY, id++};
    }

    // Initialize cell buttons
    Color cellButtonColors[] = {RAYWHITE, BEIGE, GREEN, RED, LIGHTGRAY};
    const char *cellButtonText[] = {"Empty", "Wall", "Start", "End", "Weight"};
    int cellArraySize = 5;
    Button cellButtons[cellArraySize];

    for (int i = 0; i < cellArraySize; i++)
    {
        cellButtons[i] = {CenterAndSizeGuiElement(buttonRegions[2], cellArraySize, i, gridSpaced), cellButtonText[i], buttonTypeImage, cellButtonColors[i], id++};
    }

    // Initialize control buttons
    enum ControlTypes
    {
        visualize,
        clear
    };
    const char *controlButtonText[] = {"Visualize", "Clear"};
    Color controlButtonColors[] = {PURPLE, LIGHTGRAY};
    int controlArraySize = 2;
    Button controlButtons[controlArraySize];

    for (int i = 0; i < controlArraySize; i++)
    {
        controlButtons[i] = {CenterAndSizeGuiElement(buttonRegions[3], controlArraySize, i, verticalSpaced), controlButtonText[i], buttonTypeText, controlButtonColors[i], id++};
    }

    // Initialize grid
    const int gridSize = gridRows * gridCols;
    Cell cells[gridSize];

    for (int i = 0; i < gridSize; i++)
    {
        cells[i] = {CenterAndSizeGuiElement(gridRegion, gridSize, i, gridFill), RAYWHITE, false, id++};
    }

    // Render loop
    while (!WindowShouldClose())
    {
        // Update GUI as window size changes
        if (IsWindowResized())
        {
            screenHeight = GetScreenHeight();
            screenWidth = GetScreenWidth();

            // Update control region
            controlRegion.height = screenHeight;
            controlRegion.width = screenWidth / 5;

            // Update grid region
            gridRegion.x = controlRegion.width;
            gridRegion.height = screenHeight;
            gridRegion.width = screenWidth - controlRegion.width;
            gridRegion = CenterAndSizeGuiElement(gridRegion, 1, 0, squareFill);

            // Update button regions
            for (int i = 0; i < buttonRegionArraySize; i++)
            {
                buttonRegions[i] = CenterAndSizeGuiElement(controlRegion, buttonRegionArraySize, i, verticalFill);
            }

            // Update pathfinding buttons
            for (int i = 0; i < pathfindingArraySize; i++)
            {
                pathfindingButtons[i].bounds = CenterAndSizeGuiElement(buttonRegions[0], pathfindingArraySize, i, verticalSpaced);
            }

            // Update maze buttons
            for (int i = 0; i < mazeArraySize; i++)
            {
                mazeButtons[i].bounds = CenterAndSizeGuiElement(buttonRegions[1], mazeArraySize, i, verticalSpaced);
            }

            // Update cell buttons
            for (int i = 0; i < cellArraySize; i++)
            {
                cellButtons[i].bounds = CenterAndSizeGuiElement(buttonRegions[2], cellArraySize, i, gridSpaced);
            }

            // Update control buttons
            for (int i = 0; i < controlArraySize; i++)
            {
                controlButtons[i].bounds = CenterAndSizeGuiElement(buttonRegions[3], controlArraySize, i, verticalSpaced);
            }

            // Update grid
            for (int i = 0; i < gridSize; i++)
            {
                cells[i].bounds = CenterAndSizeGuiElement(gridRegion, gridSize, i, gridFill);
            }
        }

        // Thread check
        if (std::atomic_load(&done))
        {
            // Synchronize the thread
            thread.join();
            GuiUnlock();
            std::atomic_store(&done, false);
        }

        // Draw code
        BeginDrawing();
        ClearBackground(DARKGRAY);

        // Draw grid
        for (int i = 0; i < gridSize; i++)
        {
            // Check if the cell has been clicked
            if (cells[i].Draw())
            {
                // Placing a start cell
                if (state.cellDrawColor == GREEN)
                {
                    // Start and end should not be the same cell
                    if (i != state.endCellIndex)
                    {
                        state.startCellIndex = i;
                    }
                }
                // Placing an end cell
                else if (state.cellDrawColor == RED)
                {
                    if (i != state.startCellIndex)
                    {
                        state.endCellIndex = i;
                    }
                }
                // Update the cell to the current draw color
                else
                {
                    // Special case for weighted nodes that will get tinted
                    if (state.cellDrawColor == LIGHTGRAY)
                    {
                        cells[i].color = RAYWHITE;
                        cells[i].weighted = true;
                    }
                    else
                    {
                        cells[i].color = state.cellDrawColor;
                        cells[i].weighted = false;
                    }
                }
            }

            // Draw start and end cells
            if (i == state.startCellIndex)
            {
                DrawRectangleRec(cells[i].bounds, GREEN);
                DrawRectangleLinesEx(cells[i].bounds, 1, BLACK);
            }
            else if (i == state.endCellIndex)
            {
                DrawRectangleRec(cells[i].bounds, RED);
                DrawRectangleLinesEx(cells[i].bounds, 1, BLACK);
            }
        }

        // Draw pathfinding buttons
        for (int i = 0; i < pathfindingArraySize; i++)
        {
            if (pathfindingButtons[i].Draw())
            {
                state.pathfindingButtonIndex = i;
            }
            // Highlight selected button
            if (state.pathfindingButtonIndex == i)
            {
                DrawRectangleLinesEx(pathfindingButtons[i].bounds, 2, BLUE);
            }
        }

        // Draw maze buttons
        for (int i = 0; i < mazeArraySize; i++)
        {
            if (mazeButtons[i].Draw())
            {
                // Fill the grid with walls
                for (int i = 0; i < gridSize; i++)
                {
                    cells[i].color = BEIGE;
                    cells[i].weighted = false;
                }

                switch (i)
                {
                case prim:
                {
                    GuiLock();
                    thread = std::thread(RandomizedPrimsMaze, std::ref(cells), gridRows, gridCols, std::ref(done));
                    break;
                }
                case dfs:
                {
                    GuiLock();
                    thread = std::thread(RandomizedDFSMaze, std::ref(cells), gridRows, gridCols, std::ref(done));
                    break;
                }
                case sidewinder:
                {
                    GuiLock();
                    thread = std::thread(SidewinderMaze, std::ref(cells), gridRows, gridCols, std::ref(done));
                    break;
                }
                }
            }
        }

        // Draw control buttons
        for (int i = 0; i < controlArraySize; i++)
        {
            if (controlButtons[i].Draw())
            {
                switch (i)
                {
                case visualize:
                {
                    // Reset grid
                    for (int i = 0; i < gridSize; i++)
                    {
                        if (cells[i].color != BEIGE)
                        {
                            cells[i].color = RAYWHITE;
                        }
                    }
                    switch (state.pathfindingButtonIndex)
                    {
                    case depthFirstSearch:
                    {
                        GuiLock();
                        thread = std::thread(DepthFirstSearch, std::ref(cells), gridRows, gridCols, state.startCellIndex, state.endCellIndex, std::ref(done));
                        break;
                    }
                    case breadthFirstSearch:
                    {
                        GuiLock();
                        thread = std::thread(BreadthFirstSearch, std::ref(cells), gridRows, gridCols, state.startCellIndex, state.endCellIndex, std::ref(done));
                        break;
                    }
                    case dijkstra:
                    {
                        GuiLock();
                        thread = std::thread(Dijkstra, std::ref(cells), gridRows, gridCols, state.startCellIndex, state.endCellIndex, std::ref(done));
                        break;
                    }
                    case aStar:
                    {
                        GuiLock();
                        thread = std::thread(AStar, std::ref(cells), gridRows, gridCols, state.startCellIndex, state.endCellIndex, std::ref(done));
                        break;
                    }
                    }
                    break;
                }
                case clear:
                {
                    for (int i = 0; i < gridSize; i++)
                    {
                        cells[i].color = RAYWHITE;
                        cells[i].weighted = false;
                    }
                    break;
                }
                }
            }
        }

        // Draw cell buttons
        for (int i = 0; i < cellArraySize; i++)
        {
            if (cellButtons[i].Draw())
            {
                state.cellButtonIndex = i;
                state.cellDrawColor = cellButtons[i].color;
            }
            if (state.cellButtonIndex == i)
            {
                DrawRectangleLinesEx(cellButtons[i].bounds, 2, BLUE);
            }
        }

        DrawFPS(0, 0);
        EndDrawing();
    }

    // De-Initialization
    CloseWindow();

    return 0;
}