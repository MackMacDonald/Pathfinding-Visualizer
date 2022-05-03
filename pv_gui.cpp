#include "pv_gui.h"
#include <math.h>
#include "raylib.h"

static GuiState guiState = {-1, -1, false};

void GuiLock()
{
    guiState.locked = true;
}

void GuiUnlock()
{
    guiState.locked = false;
}

bool GuiIsLocked()
{
    return guiState.locked;
}

Rectangle CenterAndSizeGuiElement(Rectangle outerBounds, int numElements, int index, GuiCenterStyle centerStyle)
{
    Rectangle bounds;

    switch (centerStyle)
    {
    // GUI elements placed vertically with space between
    case verticalSpaced:
    {
        float width = outerBounds.width * .8;
        float height = outerBounds.height * .8 / numElements;
        float xOffset = (outerBounds.width / 5) / 2;
        float yOffset = (outerBounds.height / 5) / (numElements + 1);

        float x = outerBounds.x + xOffset;
        float y = outerBounds.y + ((index + 1) * yOffset) + (index * height);

        bounds = {x, y, width, height};
        break;
    }
    // Fill the space vertically divided by the number of elements
    case verticalFill:
    {
        float height = outerBounds.height / numElements;
        float y = height * index;

        bounds = {outerBounds.x, y, outerBounds.width, height};
        break;
    }
    // GUI elements placed in a grid with space between
    case gridSpaced:
    {
        int rows;
        int cols;

        // More columns if width is larger than height
        if (outerBounds.width > outerBounds.height)
        {
            rows = floor(sqrt(numElements));
            cols = ceil(numElements / (float)rows);
        }
        else
        {
            cols = floor(sqrt(numElements));
            rows = ceil(numElements / (float)cols);
        }

        // Convert index to 2D index
        int cIndex = index % cols;
        int rIndex = floor(index / cols);

        // Align with the other button groups
        outerBounds.x = outerBounds.x + (outerBounds.width * .2 / 2);
        outerBounds.width = outerBounds.width * .8;

        float width = outerBounds.width * .8 / cols;
        float height = outerBounds.height * .8 / rows;
        float xOffset = (outerBounds.width / 5) / (cols - 1);
        float yOffset = (outerBounds.height / 5) / (rows + 1);

        float x = outerBounds.x + ((cIndex)*xOffset) + (cIndex * width);
        float y = outerBounds.y + ((rIndex + 1) * yOffset) + (rIndex * height);

        bounds = {x, y, width, height};
        break;
    }
    // Fill the space with a grid
    case gridFill:
    {
        int cols = floor(sqrt(numElements));
        int rows = ceil(numElements / (float)cols);

        // Convert index to 2D index
        int cIndex = index % cols;
        int rIndex = floor(index / cols);

        float width = outerBounds.width / cols;
        float height = outerBounds.height / rows;
        float x = outerBounds.x + (cIndex * width);
        float y = outerBounds.y + (rIndex * height);

        bounds = {x, y, width, height};
        break;
    }
    // Build largest square possible and center it
    case squareFill:
    {
        if (outerBounds.width > outerBounds.height)
        {
            float size = outerBounds.height;
            float xOffset = (outerBounds.width - size) / 2;

            bounds = {outerBounds.x + xOffset, outerBounds.y, size, size};
        }
        else
        {
            float size = outerBounds.width;
            float yOffset = (outerBounds.height - size) / 2;

            bounds = {outerBounds.x, outerBounds.y + yOffset, size, size};
        }
        break;
    }
    }
    return bounds;
}

bool Cell::Draw()
{
    // Tint weighted cells
    if (this->weighted)
    {
        this->color = Fade(this->color, .6);
    }
    else
    {
        this->color = Fade(this->color, 1);
    }

    DrawRectangleRec(this->bounds, this->color);
    DrawRectangleLinesEx(this->bounds, 1, BLACK);

    // Ignore mouse input if gui is locked
    if (!GuiIsLocked())
    {
        if (CheckCollisionPointRec(GetMousePosition(), this->bounds))
        {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                return true;
            }
        }
    }
    return false;
}

bool Button::Draw()
{
    DrawRectangleRec(this->bounds, this->color);
    DrawRectangleLinesEx(this->bounds, 2, WHITE);

    if (this->text != NULL || this->text[0] != '\0')
    {
        int textSize = MeasureText(this->text, 18) / 2;
        DrawText(this->text, this->bounds.x + this->bounds.width / 2 - textSize, this->bounds.y + this->bounds.height / 2 - 5, 18, BLACK);
    }

    // Ignore mouse input if gui is locked
    if (!GuiIsLocked())
    {
        if (CheckCollisionPointRec(GetMousePosition(), this->bounds))
        {
            guiState.hovered = this->id;

            if (guiState.interactingWith == -1)
            {
                DrawRectangleLinesEx(this->bounds, 2, YELLOW);

                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                {
                    guiState.interactingWith = this->id;
                    this->color = Fade(this->color, .6);
                }
            }

            else if (guiState.interactingWith == this->id)
            {
                DrawRectangleLinesEx(this->bounds, 2, YELLOW);

                if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
                {
                    guiState.interactingWith = -1;
                    this->color = Fade(this->color, 1);
                    return true;
                }
            }

            // Interacting with different element
            else
            {
                if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
                {
                    guiState.interactingWith = -1;
                }
            }
        }
    }

    // Reset clicked highlighting
    if (guiState.interactingWith == -1)
    {
        this->color = Fade(this->color, 1);
    }

    // Reset interacting if mouse is up
    if (this->id == guiState.interactingWith)
    {
        if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
        {
            guiState.interactingWith = -1;
        }
    }

    return false;
}
