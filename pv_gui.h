#ifndef PV_GUI_H
#define PV_GUI_H

#include "raylib.h"

enum GuiButtonType
{
    buttonTypeText,
    buttonTypeImage
};

enum GuiCenterStyle
{
    verticalSpaced,
    verticalFill,
    gridSpaced,
    gridFill,
    squareFill
};

struct GuiState
{
    int interactingWith;
    int hovered;

    bool locked;
};

struct Cell
{
    Rectangle bounds;
    Color color;
    bool weighted;
    int id;

    bool Draw();
};

struct Button
{
    Rectangle bounds;
    const char *text;
    GuiButtonType type;
    Color color;
    int id;

    bool Draw();
};

// Centers the gui element with respect to the outer bounds based on the number of elements inside and the center style
Rectangle CenterAndSizeGuiElement(Rectangle outerBounds, int numElements, int index, GuiCenterStyle centerStyle);

void GuiLock();
void GuiUnlock();
bool GuiIsLocked();

#endif