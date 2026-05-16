#pragma once
#include "imgui.h"
#include "config.h"

typedef unsigned int GLuint;

inline GLuint GetHeroIcon(int heroId) {
    // Implementasi dummy: kembalikan 0 jika tidak ada gambar ikon yang dimuat
    return 0;
}

// ============================================================
// Unity Native Camera API
// ============================================================
#include "../Il2CppResolver.h"

inline void* GetCameraMain() {
    static void* get_main_method = nullptr;
    if (!get_main_method) {
        get_main_method = Il2CppGetMethodOffset("UnityEngine.CoreModule.dll", "UnityEngine", "Camera", "get_main", 0);
        if (!get_main_method) {
            get_main_method = Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Camera", "get_main", 0);
        }
    }
    if (get_main_method) {
        return reinterpret_cast<void*(*)()>(get_main_method)();
    }
    return nullptr;
}

inline bool UnityWorldToScreen(void* camera, const Vec3& world, Vec2& screen, float screenH) {
    if (!camera) return false;
    
    static void* w2s_method = nullptr;
    if (!w2s_method) {
        w2s_method = Il2CppGetMethodOffset("UnityEngine.CoreModule.dll", "UnityEngine", "Camera", "WorldToScreenPoint", 1);
        if (!w2s_method) {
            w2s_method = Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Camera", "WorldToScreenPoint", 1);
        }
    }
    if (w2s_method) {
        // Panggil method asli dari Unity Engine
        Vec3 result = reinterpret_cast<Vec3(*)(void*, Vec3)>(w2s_method)(camera, world);
        
        // Z < 0 berarti objek ada di belakang kamera
        if (result.z < 0.01f) return false; 
        
        screen.x = result.x;
        // ImGui menggunakan Top-Left (0,0), sedangkan Unity menggunakan Bottom-Left (0,0)
        screen.y = screenH - result.y; 
        return true;
    }
    return false;
}

// Forward declaration untuk menghindari circular dependency
void DrawPlayerESP(ImDrawList* draw, void* camera, float screenW, float screenH, bool hasSelf, const ImVec2& selfPosVec2);

inline void RenderESPCore() {
    // Dapatkan instance Camera.main dari Unity
    void* cameraMain = GetCameraMain();
    
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    
    // Memanggil fungsi ESP dari esp_player.h
    DrawPlayerESP(drawList, cameraMain, io.DisplaySize.x, io.DisplaySize.y, false, ImVec2(0,0));
}

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
