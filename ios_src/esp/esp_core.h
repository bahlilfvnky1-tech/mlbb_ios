#pragma once
#include "imgui.h"
#include "config.h"

typedef unsigned int GLuint;

extern GLuint GetHeroIcon(int heroId);

inline float GetDynamicOffset(float screenHeight, float fovScale) {
    float baseOffset = screenHeight / 6.5f;
    return baseOffset / fovScale;
}

inline ImU32 GetHealthColor(int health, int max_health) {
    float healthPercent = (float)health / (float)max_health;
    if (healthPercent > 0.75f) {
        return IM_COL32(50, 255, 50, 255);
    } else if (healthPercent > 0.5f) {
        int green = 255;
        int red = (int)(255 * (1.0f - (healthPercent - 0.5f) * 2.0f));
        return IM_COL32(red, green, 50, 255);
    } else if (healthPercent > 0.25f) {
        int red = 255;
        int green = (int)(255 * (healthPercent * 2.0f));
        return IM_COL32(red, green, 50, 255);
    } else {
        return IM_COL32(255, 50, 50, 255);
    }
}
