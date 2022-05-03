#include "raylib.h"
#include "pv_gui.h"

#include <atomic>
#include <chrono>
#include <thread>

#define MAX 2500
#define DELAY 10

struct Node
{
    int index;
    int parentIndex;
    int distance;
};

struct MinHeap
{
    int size;
    Node elements[MAX];

    void Insert(Node n);
    Node ExtractMin();
    void DecreasePriority(Node n, int priority);
    void MinHeapify(int index);
    bool Find(int gridIndex);
    bool IsEmpty();
    int Parent(int index);
    int LeftChild(int index);
    int RightChild(int index);
    void Swap(Node *x, Node *y);
};

int MinHeap::Parent(int index)
{
    return (index - 1) / 2;
}

int MinHeap::LeftChild(int index)
{
    return (2 * index) + 1;
}

int MinHeap::RightChild(int index)
{
    return (2 * index) + 2;
}

void MinHeap::Swap(Node *x, Node *y)
{
    Node temp = *x;
    *x = *y;
    *y = temp;
}

void MinHeap::DecreasePriority(Node n, int priority)
{
    int index = 0;
    for (int i = 0; i < size; i++)
    {
        if (elements[i].index == n.index)
        {
            index = i;
            elements[i].distance = priority;
        }
    }

    while (index != 0 && elements[Parent(index)].distance > elements[index].distance)
    {
        Swap(&elements[index], &elements[Parent(index)]);
        index = Parent(index);
    }
}

void MinHeap::MinHeapify(int index)
{
    bool done = false;

    while (!done)
    {
        int left = LeftChild(index);
        int right = RightChild(index);
        int smallest = index;

        if (left < size && elements[left].distance < elements[smallest].distance)
        {
            smallest = left;
        }
        if (right < size && elements[right].distance < elements[smallest].distance)
        {
            smallest = right;
        }

        if (smallest != index)
        {
            Swap(&elements[index], &elements[smallest]);
            index = smallest;
        }
        else
        {
            done = true;
        }
    }
}

Node MinHeap::ExtractMin()
{
    if (size == 1)
    {
        size--;
        return elements[0];
    }

    Node root = elements[0];
    elements[0] = elements[size - 1];
    size--;
    MinHeapify(0);

    return root;
}

bool MinHeap::Find(int gridIndex)
{
    for (int i = 0; i < size; i++)
    {
        if (elements[i].index == gridIndex)
        {
            return true;
        }
    }
    return false;
}

void MinHeap::Insert(Node n)
{
    size++;
    int index = size - 1;
    elements[index] = n;

    while (index != 0 && elements[Parent(index)].distance > elements[index].distance)
    {
        Swap(&elements[index], &elements[Parent(index)]);
        index = Parent(index);
    }
}

bool MinHeap::IsEmpty()
{
    return (size < 1);
}

struct Stack
{
    int top = 0;
    Node elements[MAX];

    Node Pop();
    bool Push(Node item);
    bool IsEmpty();
    void RemoveAtIndex(int index);
};

Node Stack::Pop()
{
    return elements[top--];
}

bool Stack::Push(Node item)
{
    if (top >= (MAX - 1))
    {
        return false;
    }
    else
    {
        elements[++top] = item;
        return true;
    }
}

bool Stack::IsEmpty()
{
    return (top < 0);
}

void Stack::RemoveAtIndex(int index)
{
    Stack temp = {-1};

    while (!IsEmpty())
    {
        if (top != index)
        {
            temp.Push(Pop());
        }
        else
        {
            Pop();
        }
    }

    while (!temp.IsEmpty())
    {
        Push(temp.Pop());
    }
}

struct Queue
{
    int size;
    int front;
    int rear;
    Node elements[MAX];

    Node Pop();
    bool Push(Node item);
    bool IsEmpty();
};

Node Queue::Pop()
{
    size--;
    return elements[++front];
}

bool Queue::Push(Node item)
{
    if (size >= MAX)
    {
        return false;
    }
    else
    {
        elements[++rear] = item;
        size++;
        return true;
    }
}

bool Queue::IsEmpty()
{
    return (size == 0);
}

struct Coordinates
{
    int x;
    int y;
};

bool operator==(Color c1, Color c2)
{
    return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b;
}

bool operator!=(Color c1, Color c2)
{
    return !(c1 == c2);
}

int Absolute(int i)
{
    return i < 0 ? -i : i;
}

int ManhattanDistance(Coordinates a, Coordinates b)
{
    return (Absolute(a.x - b.x) + Absolute(a.y - b.y));
}

Coordinates IndexToGridIndexes(int index, int cols)
{
    int x = index % cols;
    int y = index / cols;
    Coordinates gridIndexes = {x, y};

    return gridIndexes;
}

int GridIndexesToIndex(Coordinates gridIndexes, int cols)
{
    return (gridIndexes.y * cols) + gridIndexes.x;
}

void GetNeighbourIndexes(int index, int cols, int rows, int *array)
{
    Coordinates gridIndexes = IndexToGridIndexes(index, cols);
    Coordinates neighbour;

    // Up
    if (gridIndexes.y > 0)
    {
        neighbour.x = gridIndexes.x;
        neighbour.y = gridIndexes.y - 1;
        index = GridIndexesToIndex(neighbour, cols);

        array[0] = index;
    }
    // Left
    if (gridIndexes.x > 0)
    {
        neighbour.x = gridIndexes.x - 1;
        neighbour.y = gridIndexes.y;
        index = GridIndexesToIndex(neighbour, cols);

        array[1] = index;
    }
    // Down
    if (gridIndexes.y < rows - 1)
    {
        neighbour.x = gridIndexes.x;
        neighbour.y = gridIndexes.y + 1;
        index = GridIndexesToIndex(neighbour, cols);

        array[2] = index;
    }
    // Right
    if (gridIndexes.x < cols - 1)
    {
        neighbour.x = gridIndexes.x + 1;
        neighbour.y = gridIndexes.y;
        index = GridIndexesToIndex(neighbour, cols);

        array[3] = index;
    }
}

void DepthFirstSearch(Cell cells[], int rows, int cols, int startIndex, int endIndex, std::atomic<bool> &done)
{
    Node start = {startIndex, -1};
    Stack stack;
    Node visited[MAX];
    bool solved = false;

    stack.Push(start);
    visited[startIndex] = start;
    cells[startIndex].color = SKYBLUE;

    while (!stack.IsEmpty())
    {
        Node current = stack.Pop();

        int indexes[4] = {-1, -1, -1, -1};

        GetNeighbourIndexes(current.index, cols, rows, indexes);

        for (int i = 0; i < 4; i++)
        {
            if (indexes[i] != -1 && cells[indexes[i]].color != BEIGE && cells[indexes[i]].color != SKYBLUE)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
                cells[indexes[i]].color = SKYBLUE;
                Node neighbour = {indexes[i], current.index};
                visited[indexes[i]] = neighbour;
                stack.Push(neighbour);
                if (indexes[i] == endIndex)
                {
                    solved = true;
                    break;
                }
            }
        }

        // Reached the end
        if (solved)
        {
            // Solved path
            current = visited[endIndex];
            cells[current.index].color = GOLD;
            while (current.parentIndex != -1)
            {
                current = visited[current.parentIndex];
                std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
                cells[current.index].color = GOLD;
            }
            break;
        }
    }
    std::atomic_store(&done, true);
}

void BreadthFirstSearch(Cell cells[], int rows, int cols, int startIndex, int endIndex, std::atomic<bool> &done)
{
    Node start = {startIndex, -1};
    Queue queue = {0, -1, -1};
    Node visited[MAX];
    bool solved = false;

    queue.Push(start);
    visited[startIndex] = start;
    cells[startIndex].color = SKYBLUE;

    while (!queue.IsEmpty())
    {
        Node current = queue.Pop();

        int indexes[4] = {-1, -1, -1, -1};

        GetNeighbourIndexes(current.index, cols, rows, indexes);

        for (int i = 0; i < 4; i++)
        {
            if (indexes[i] != -1 && cells[indexes[i]].color != BEIGE && cells[indexes[i]].color != SKYBLUE)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
                cells[indexes[i]].color = SKYBLUE;
                Node neighbour = {indexes[i], current.index};
                visited[indexes[i]] = neighbour;
                queue.Push(neighbour);
                if (indexes[i] == endIndex)
                {
                    solved = true;
                    break;
                }
            }
        }

        // Reached the end
        if (solved)
        {
            // Solved path
            current = visited[endIndex];
            cells[current.index].color = GOLD;
            while (current.parentIndex != -1)
            {
                current = visited[current.parentIndex];
                std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
                cells[current.index].color = GOLD;
            }
            break;
        }
    }
    std::atomic_store(&done, true);
}

void Dijkstra(Cell cells[], int rows, int cols, int startIndex, int endIndex, std::atomic<bool> &done)
{
    Node start = {startIndex, -1, 0};
    MinHeap minHeap = {0};
    Node visited[MAX];
    bool solved = false;

    for (int i = 0; i < MAX; i++)
    {
        visited[i].distance = INT_MAX;
        visited[i].parentIndex = -1;
    }

    minHeap.Insert(start);
    visited[startIndex] = start;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
    cells[startIndex].color = SKYBLUE;

    while (!minHeap.IsEmpty())
    {
        Node current = minHeap.ExtractMin();

        int indexes[4] = {-1, -1, -1, -1};
        GetNeighbourIndexes(current.index, cols, rows, indexes);

        for (int i = 0; i < 4; i++)
        {
            int neighbourIndex = indexes[i];
            if (neighbourIndex != -1 && cells[neighbourIndex].color != BEIGE)
            {
                int cost = 1;
                if (cells[neighbourIndex].weighted)
                {
                    cost = 10;
                }
                int newDistance = current.distance + cost;

                if (newDistance < visited[neighbourIndex].distance)
                {

                    Node neighbour = {neighbourIndex, current.index, newDistance};
                    visited[neighbourIndex] = neighbour;

                    if (minHeap.Find(neighbourIndex))
                    {
                        minHeap.DecreasePriority(neighbour, newDistance);
                    }
                    else
                    {
                        minHeap.Insert(neighbour);
                        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
                        cells[neighbourIndex].color = SKYBLUE;
                    }

                    if (neighbourIndex == endIndex)
                    {
                        solved = true;
                        break;
                    }
                }
            }
        }
        // Reached the end
        if (solved)
        {
            // Solved path
            current = visited[endIndex];
            cells[current.index].color = GOLD;
            while (current.parentIndex != -1)
            {
                current = visited[current.parentIndex];
                std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
                cells[current.index].color = GOLD;
            }
            break;
        }
    }
    std::atomic_store(&done, true);
}

void AStar(Cell cells[], int rows, int cols, int startIndex, int endIndex, std::atomic<bool> &done)
{
    Node start = {startIndex, -1, 0};
    MinHeap minHeap = {0};
    Node visited[MAX];
    Node gScore[MAX];
    Node fScore[MAX];
    bool solved = false;

    for (int i = 0; i < MAX; i++)
    {
        gScore[i].distance = INT_MAX;
        fScore[i].distance = INT_MAX;
        gScore[i].parentIndex = -1;
        fScore[i].parentIndex = -1;
    }

    minHeap.Insert(start);
    visited[startIndex] = start;
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
    cells[startIndex].color = SKYBLUE;

    gScore[startIndex].distance = 0;
    Coordinates startCoord = IndexToGridIndexes(startIndex, cols);
    Coordinates endCoord = IndexToGridIndexes(endIndex, cols);
    fScore[startIndex].distance = ManhattanDistance(startCoord, endCoord);

    while (!minHeap.IsEmpty())
    {
        Node current = minHeap.ExtractMin();

        int indexes[4] = {-1, -1, -1, -1};
        GetNeighbourIndexes(current.index, cols, rows, indexes);

        for (int i = 0; i < 4; i++)
        {
            int neighbourIndex = indexes[i];
            if (neighbourIndex != -1 && cells[neighbourIndex].color != BEIGE)
            {
                int cost = 1;
                if (cells[neighbourIndex].weighted)
                {
                    cost = 10;
                }
                int tentativeScore = gScore[current.index].distance + cost;

                if (tentativeScore < gScore[neighbourIndex].distance)
                {

                    Node gNeighbour = {neighbourIndex, current.index, tentativeScore};
                    visited[neighbourIndex] = gNeighbour;
                    gScore[neighbourIndex] = gNeighbour;

                    Coordinates start = IndexToGridIndexes(neighbourIndex, cols);
                    Coordinates goal = IndexToGridIndexes(endIndex, cols);
                    int estimatedCostToGoal = ManhattanDistance(start, goal);

                    Node fNeighbour = {neighbourIndex, current.index, tentativeScore + estimatedCostToGoal};
                    fScore[neighbourIndex] = fNeighbour;

                    if (!minHeap.Find(neighbourIndex))
                    {
                        minHeap.Insert(fNeighbour);
                        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
                        cells[neighbourIndex].color = SKYBLUE;
                    }

                    if (neighbourIndex == endIndex)
                    {
                        solved = true;
                        break;
                    }
                }
            }
        }
        // Reached the end
        if (solved)
        {
            // Solved path
            current = visited[endIndex];
            cells[current.index].color = GOLD;
            while (current.parentIndex != -1)
            {
                current = visited[current.parentIndex];
                std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
                cells[current.index].color = GOLD;
            }
            break;
        }
    }
    std::atomic_store(&done, true);
}

void RandomizedPrimsMaze(Cell cells[], int rows, int cols, std::atomic<bool> &done)
{
    int size = rows * cols;

    // Start with grid filled with walls and keep track of walls being processed
    Stack wallList = {-1};

    // Random number for start cell
    SetRandomSeed(GetTime());
    int randStartCell = GetRandomValue(0, size - 1);

    // Mark random start cell as maze path
    cells[randStartCell].color = RAYWHITE;

    int indexes[4] = {-1, -1, -1, -1};

    GetNeighbourIndexes(randStartCell, cols, rows, indexes);

    for (int i = 0; i < 4; i++)
    {
        if (indexes[i] != -1)
        {
            Node wall = {indexes[i]};
            wallList.Push(wall);
        }
    }

    while (!wallList.IsEmpty())
    {
        int randomWall = GetRandomValue(0, wallList.top);

        int index = wallList.elements[randomWall].index;
        int neighbourPathCount = 0;

        int indexes[4] = {-1, -1, -1, -1};
        GetNeighbourIndexes(index, cols, rows, indexes);

        for (int i = 0; i < 4; i++)
        {
            if (cells[indexes[i]].color == RAYWHITE)
            {
                neighbourPathCount++;
                indexes[i] = -1;
            }
        }

        if (neighbourPathCount <= 1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
            cells[index].color = RAYWHITE;
            for (int i = 0; i < 4; i++)
            {
                if (indexes[i] != -1)
                {
                    Node wall = {indexes[i]};
                    wallList.Push(wall);
                }
            }
        }

        // Remove wall from stack
        wallList.RemoveAtIndex(randomWall);
    }

    std::atomic_store(&done, true);
}

void RandomizedDFSMaze(Cell cells[], int rows, int cols, std::atomic<bool> &done)
{
    int size = rows * cols;

    // Start with grid filled with walls and keep track of walls being processed
    Stack wallList = {-1};

    // Random number for start cell
    SetRandomSeed(GetTime());
    int randStartCell = GetRandomValue(0, size - 1);

    // Mark random start cell as maze path
    cells[randStartCell].color = RAYWHITE;

    Node randStartNode = {randStartCell};

    wallList.Push(randStartNode);

    while (!wallList.IsEmpty())
    {
        Node node = wallList.Pop();

        int indexes[4] = {-1, -1, -1, -1};
        GetNeighbourIndexes(node.index, cols, rows, indexes);

        int availableNeighbours = 4;
        for (int i = 0; i < 4; i++)
        {
            if (cells[indexes[i]].color == RAYWHITE || indexes[i] == -1)
            {
                availableNeighbours--;
                indexes[i] = -1;
            }
            // Check if neighbour would create a loop
            else
            {
                int neighbourIndexes[4] = {-1, -1, -1, -1};
                GetNeighbourIndexes(indexes[i], cols, rows, neighbourIndexes);

                int neighbourPathCount = 0;
                for (int i = 0; i < 4; i++)
                {
                    if (cells[neighbourIndexes[i]].color == RAYWHITE)
                    {
                        neighbourPathCount++;
                        neighbourIndexes[i] = -1;
                    }
                }

                if (neighbourPathCount > 1)
                {
                    availableNeighbours--;
                    indexes[i] = -1;
                }
            }
        }

        if (availableNeighbours > 0)
        {
            wallList.Push(node);
            int randIndex = GetRandomValue(0, availableNeighbours - 1);
            int currIndex = 0;
            int foundIndex = 0;

            for (int i = 0; i < 4; i++)
            {
                if (cells[indexes[i]].color != RAYWHITE && indexes[i] != -1)
                {
                    if (currIndex == randIndex)
                    {
                        foundIndex = i;
                    }
                    currIndex++;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
            cells[indexes[foundIndex]].color = RAYWHITE;
            Node wall = {indexes[foundIndex]};
            wallList.Push(wall);
        }
    }
    std::atomic_store(&done, true);
}

void SidewinderMaze(Cell cells[], int rows, int cols, std::atomic<bool> &done)
{
    SetRandomSeed(GetTime());

    // First row path
    for (int i = 0; i < cols; i++)
    {
        cells[i].color = RAYWHITE;
    }

    for (int i = 2; i < rows; i += 2)
    {
        int runStartIndex = 0;
        bool skipNext = false;

        for (int j = 0; j < cols; j++)
        {

            if (!skipNext)
            {
                // Current cell is a path
                Coordinates coords = {j, i};
                int index = GridIndexesToIndex(coords, cols);
                std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
                cells[index].color = RAYWHITE;

                int goEast = GetRandomValue(0, 1);

                // Edge of the maze
                if (j == cols - 1)
                {
                    goEast = false;
                }

                if (j + 1 >= cols || !goEast)
                {
                    int availableNodes[cols];
                    int numAvailable = 0;

                    for (int x = runStartIndex; x < j + 1; x++)
                    {
                        coords = {x, i - 2};
                        index = GridIndexesToIndex(coords, cols);
                        if (cells[index].color == RAYWHITE)
                        {
                            availableNodes[numAvailable++] = x;
                        }
                    }

                    if (numAvailable == 0)
                    {
                        // Make sure edge of the maze is connected
                        if (j == cols - 1)
                        {
                            coords = {j - 1, i};
                            index = GridIndexesToIndex(coords, cols);
                            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
                            cells[index].color = RAYWHITE;
                        }
                        continue;
                    }

                    int randIndex = GetRandomValue(0, numAvailable - 1);

                    // Path north of a cell that has been visited in this row
                    coords = {availableNodes[randIndex], i - 1};
                    index = GridIndexesToIndex(coords, cols);
                    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
                    cells[index].color = RAYWHITE;

                    runStartIndex = j + 2;
                    skipNext = true;
                }
            }
            else
            {
                skipNext = false;
            }
        }
    }
    std::atomic_store(&done, true);
}
