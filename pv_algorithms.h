#ifndef PV_ALGORITHMS_H
#define PV_ALGORITHMS_H

#include "raylib.h"
#include "pv_gui.h"

#include <atomic>

void RandomizedPrimsMaze(Cell cells[], int rows, int cols, std::atomic<bool> &done);
void RandomizedDFSMaze(Cell cells[], int rows, int cols, std::atomic<bool> &done);
void SidewinderMaze(Cell cells[], int rows, int cols, std::atomic<bool> &done);

void DepthFirstSearch(Cell cells[], int rows, int cols, int start, int end, std::atomic<bool> &done);
void BreadthFirstSearch(Cell cells[], int rows, int cols, int startIndex, int endIndex, std::atomic<bool> &done);
void Dijkstra(Cell cells[], int rows, int cols, int startIndex, int endIndex, std::atomic<bool> &done);
void AStar(Cell cells[], int rows, int cols, int startIndex, int endIndex, std::atomic<bool> &done);

bool operator==(Color c1, Color c2);
bool operator!=(Color c1, Color c2);

#endif