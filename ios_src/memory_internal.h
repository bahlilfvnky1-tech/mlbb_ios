#pragma once
#include <stdint.h>
#include <string.h>

// Dalam arsitektur internal (iOS dylib), kita memodifikasi kelas Memory
// agar tidak membaca file /proc/pid/mem, melainkan menggunakan pointer langsung.
// Hal ini membuat performa baca memori menjadi instan (0 ms delay).

extern uintptr_t g_il2cppBase;

class InternalMemory {
public:
    template<typename T>
    static T Read(uintptr_t address) {
        // Validasi dasar agar game tidak crash jika pointer kosong
        if (address < 0x100000000) return T{}; 
        return *(T*)address;
    }
    
    static bool ReadRaw(uintptr_t address, void* buffer, size_t size) {
        if (address < 0x100000000) return false;
        memcpy(buffer, (void*)address, size);
        return true;
    }

    // Karena ini internal cheat, menulis memori langsung bisa dilakukan dengan mudah
    template<typename T>
    static bool Write(uintptr_t address, T value) {
        if (address < 0x100000000) return false;
        *(T*)address = value;
        return true;
    }
};
