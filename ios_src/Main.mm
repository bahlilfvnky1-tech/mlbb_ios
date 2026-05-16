#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <mach-o/dyld.h>
#include <pthread.h>
#include <dlfcn.h>
#include "memory_internal.h"
#include "esp/entity.h"

#import "ImGuiOverlay.h"

// Deklarasi global base address
uintptr_t g_il2cppBase = 0;

// Fungsi untuk mencari Base Address dari UnityFramework
void FindBaseAddress() {
    for (uint32_t i = 0; i < _dyld_image_count(); i++) {
        const char *name = _dyld_get_image_name(i);
        if (strstr(name, "UnityFramework")) {
            g_il2cppBase = _dyld_get_image_vmaddr_slide(i) + 0x100000000;
            NSLog(@"[Cheat] Found UnityFramework Base: 0x%lx", g_il2cppBase);
            break;
        }
    }
}

// Thread pembaca memori (Scanner)
void* MemoryThread(void* arg) {
    NSLog(@"[Cheat] Memory Thread Started!");
    
    // Tunggu sampai game termuat sempurna
    sleep(10);
    FindBaseAddress();
    
    while(true) {
        if (g_il2cppBase > 0) {
            uintptr_t logicBmPtr = InternalMemory::Read<uintptr_t>(g_il2cppBase + InternalMemory::OFF_LOGIC_BATTLE_MANAGER);
            uintptr_t logicBmIns = InternalMemory::Read<uintptr_t>(logicBmPtr + 0xB8);
            
            uintptr_t bmPtr = InternalMemory::Read<uintptr_t>(g_il2cppBase + InternalMemory::OFF_BATTLE_MANAGER);
            uintptr_t bmIns = InternalMemory::Read<uintptr_t>(bmPtr + 0xB8);
            
            if (bmIns && logicBmIns) {
                g_Battle.Update(bmIns, logicBmIns);
            }
        }
        usleep(30000); // 30ms sleep (~33 fps ESP update rate)
    }
    
    return NULL;
}

// Atribut constructor menjamin fungsi ini dipanggil otomatis 
// sesaat setelah libmlbb_cheat.dylib berhasil di-load oleh iOS
__attribute__((constructor))
void InitCheat() {
    NSLog(@"[Cheat] Dylib Injected Successfully!");
    
    // Setup UI di Main Thread (Harus di main thread karena UIKit)
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        NSLog(@"[Cheat] Initializing UI Overlay...");
        [ImGuiOverlay sharedOverlay];
    });

    // Jalankan Memory Scanner di background thread
    pthread_t ptid;
    pthread_create(&ptid, NULL, MemoryThread, NULL);
}
