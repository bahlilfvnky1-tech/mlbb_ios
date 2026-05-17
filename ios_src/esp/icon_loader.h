#pragma once
// ============================================================
//  ICON LOADER — iOS Port dari Android DrawIcon.h
//  Base64 decode → stb_image → OpenGL ES texture
//  Path iOS: /var/mobile/Documents/mlbb_esp/
// ============================================================
#include <string>
#include <vector>
#include <fstream>

// ---- stb_image (single-header) ----
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ---- base64 decoder ----
#include "base64.hpp"

// ---- Icon data (base64 encoded PNGs dari Android referensi) ----
#include "ICON.h"        // iconHeroList[]
#include "IconList.h"    // ICTexture(), MonsterTexture(), struct Icon, HeroIcon[], MonsterIcon[]

// ==============================================================
//  Helper: tulis ke file sementara di iOS Documents
// ==============================================================
static const char* kTmpDir = "/var/mobile/Documents/mlbb_esp/";

static void EnsureDir() {
    mkdir(kTmpDir, 0755);
}

static void WriteToFile(const std::string& path, const std::string& data) {
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    f.write(data.c_str(), data.size());
}

// ==============================================================
//  Buat OpenGL ES texture dari raw PNG data
// ==============================================================
static bool LoadTextureFromMemory(const unsigned char* data, int dataSize, Icon* out) {
    int w = 0, h = 0;
    unsigned char* img = stbi_load_from_memory(data, dataSize, &w, &h, nullptr, 4);
    if (!img) return false;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    stbi_image_free(img);

    out->texture = tex;
    out->width   = w;
    out->height  = h;
    out->IsValid = (tex != 0);
    return out->IsValid;
}

static bool LoadTextureFromFile(const char* path, Icon* out) {
    int w = 0, h = 0;
    unsigned char* img = stbi_load(path, &w, &h, nullptr, 4);
    if (!img) return false;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    stbi_image_free(img);

    out->texture = tex;
    out->width   = w;
    out->height  = h;
    out->IsValid = (tex != 0);
    return out->IsValid;
}

// ==============================================================
//  Load semua hero icons dari ICON.h (iconHeroList[])
// ==============================================================
static bool s_HeroIconLoaded = false;

static void AttachHeroIcons() {
    if (s_HeroIconLoaded) return;
    s_HeroIconLoaded = true;

    EnsureDir();
    int size = sizeof(iconHeroList) / sizeof(iconHeroList[0]);
    HeroIcon.resize(size);

    for (int i = 0; i < size; i++) {
        std::string path = std::string(kTmpDir) + "hero_" + std::to_string(i) + ".png";
        std::string decoded = base64::from_base64(iconHeroList[i]);
        WriteToFile(path, decoded);

        Icon t;
        LoadTextureFromFile(path.c_str(), &t);
        std::remove(path.c_str()); // hapus file temp
        HeroIcon[i] = t;
    }
}

// ==============================================================
//  Load semua monster icons dari MonsterIcon.h (MonsterList[])
// ==============================================================
static bool s_MonsterIconLoaded = false;

static void AttachMonsterIcons() {
    if (s_MonsterIconLoaded) return;
    s_MonsterIconLoaded = true;

    EnsureDir();
    int size = sizeof(MonsterList) / sizeof(MonsterList[0]);
    MonsterIcon.resize(size);

    for (int i = 0; i < size; i++) {
        std::string path = std::string(kTmpDir) + "mon_" + std::to_string(i) + ".png";
        std::string decoded = base64::from_base64(MonsterList[i]);
        WriteToFile(path, decoded);

        Icon t;
        LoadTextureFromFile(path.c_str(), &t);
        std::remove(path.c_str());
        MonsterIcon[i] = t;
    }
}

// ==============================================================
//  Public init — panggil SEKALI dari thread GL (ImGui frame pertama)
// ==============================================================
inline void InitAllIcons() {
    static bool done = false;
    if (done) return;
    done = true;

    AttachHeroIcons();
    AttachMonsterIcons();
}
