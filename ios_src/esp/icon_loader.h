#pragma once
// ============================================================
//  ICON LOADER — iOS Metal Version
//  Base64 decode → stb_image → MTLTexture (Metal backend)
//  ImGui Metal backend: ImTextureID = (__bridge void*)id<MTLTexture>
// ============================================================

#ifdef __OBJC__

#import <Metal/Metal.h>
#import <Foundation/Foundation.h>
#include <string>
#include <vector>
#include <fstream>
#include <sys/stat.h>

// ---- stb_image (single-header, only define once) ----
#ifndef ICON_LOADER_STB_DONE
#define ICON_LOADER_STB_DONE
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb_image.h"

// ---- base64 decoder ----
#include "base64.hpp"

// ---- Icon data + struct (ICON.h/MonsterIcon.h sudah #pragma once) ----
#include "IconList.h"

// ==============================================================
//  Helpers
// ==============================================================
static const char* kTmpDir = "/var/mobile/Documents/mlbb_esp/";

static void EnsureDir() {
    struct stat st;
    if (stat(kTmpDir, &st) != 0) {
        mkdir(kTmpDir, 0755);
    }
}

static void WriteToFile(const std::string& path, const std::string& data) {
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (f.is_open()) f.write(data.data(), (std::streamsize)data.size());
}

// ==============================================================
//  Buat MTLTexture dari raw RGBA byte data (stb_image output)
//  Return: void* = (__bridge_retained void*)id<MTLTexture>
//          Caller harus retain manual (ARC tidak tahu tentang ini)
// ==============================================================
static void* CreateMetalTexture(const unsigned char* pngBytes, int pngLen) {
    if (!pngBytes || pngLen <= 0) return nullptr;
    
    int w = 0, h = 0;
    unsigned char* img = stbi_load_from_memory(pngBytes, pngLen, &w, &h, nullptr, 4);
    if (!img) return nullptr;
    
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) { stbi_image_free(img); return nullptr; }
    
    MTLTextureDescriptor* desc = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                     width:(NSUInteger)w
                                    height:(NSUInteger)h
                                 mipmapped:NO];
    desc.usage = MTLTextureUsageShaderRead;
    
    id<MTLTexture> tex = [device newTextureWithDescriptor:desc];
    if (!tex) { stbi_image_free(img); return nullptr; }
    
    [tex replaceRegion:MTLRegionMake2D(0, 0, (NSUInteger)w, (NSUInteger)h)
           mipmapLevel:0
             withBytes:img
           bytesPerRow:(NSUInteger)(4 * w)];
    stbi_image_free(img);
    
    // __bridge_retained: transfer ownership ke raw pointer, ARC tidak release
    return (__bridge_retained void*)tex;
}

static void* LoadTextureFromFile(const char* path) {
    int w = 0, h = 0;
    unsigned char* img = stbi_load(path, &w, &h, nullptr, 4);
    if (!img) return nullptr;
    
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) { stbi_image_free(img); return nullptr; }
    
    MTLTextureDescriptor* desc = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                     width:(NSUInteger)w
                                    height:(NSUInteger)h
                                 mipmapped:NO];
    desc.usage = MTLTextureUsageShaderRead;
    
    id<MTLTexture> tex = [device newTextureWithDescriptor:desc];
    if (!tex) { stbi_image_free(img); return nullptr; }
    
    [tex replaceRegion:MTLRegionMake2D(0, 0, (NSUInteger)w, (NSUInteger)h)
           mipmapLevel:0
             withBytes:img
           bytesPerRow:(NSUInteger)(4 * w)];
    stbi_image_free(img);
    
    return (__bridge_retained void*)tex;
}

// ==============================================================
//  Load hero icons dari iconHeroList[] (ICON.h)
// ==============================================================
static bool s_HeroIconLoaded = false;

static void AttachHeroIcons() {
    if (s_HeroIconLoaded) return;
    s_HeroIconLoaded = true;
    
    EnsureDir();
    int size = (int)(sizeof(iconHeroList) / sizeof(iconHeroList[0]));
    HeroIcon.resize(size);
    
    for (int i = 0; i < size; i++) {
        std::string path = std::string(kTmpDir) + "hero_" + std::to_string(i) + ".png";
        std::string decoded = base64::from_base64(iconHeroList[i]);
        WriteToFile(path, decoded);
        
        Icon ic;
        ic.texture = LoadTextureFromFile(path.c_str());
        ic.IsValid = (ic.texture != nullptr);
        std::remove(path.c_str());
        HeroIcon[i] = ic;
    }
}

// ==============================================================
//  Load monster icons dari MonsterList[] (MonsterIcon.h)
// ==============================================================
static bool s_MonsterIconLoaded = false;

static void AttachMonsterIcons() {
    if (s_MonsterIconLoaded) return;
    s_MonsterIconLoaded = true;
    
    EnsureDir();
    int size = (int)(sizeof(MonsterList) / sizeof(MonsterList[0]));
    MonsterIcon.resize(size);
    
    for (int i = 0; i < size; i++) {
        std::string path = std::string(kTmpDir) + "mon_" + std::to_string(i) + ".png";
        std::string decoded = base64::from_base64(MonsterList[i]);
        WriteToFile(path, decoded);
        
        Icon ic;
        ic.texture = LoadTextureFromFile(path.c_str());
        ic.IsValid = (ic.texture != nullptr);
        std::remove(path.c_str());
        MonsterIcon[i] = ic;
    }
}

// ==============================================================
//  Public init — panggil SEKALI dari thread Metal/ImGui
// ==============================================================
inline void InitAllIcons() {
    static bool done = false;
    if (done) return;
    done = true;
    AttachHeroIcons();
    AttachMonsterIcons();
}

#else
// Non-ObjC fallback — stub
inline void InitAllIcons() {}
#endif // __OBJC__
