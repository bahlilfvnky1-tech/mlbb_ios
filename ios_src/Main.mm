#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <mach-o/dyld.h>
#include <pthread.h>
#include <dlfcn.h>
#include "memory_internal.h"
#include "esp/entity.h"

#import "ImGuiOverlay.h"

#include "Il2CppResolver.h"

// Thread pembaca memori (Scanner)
void* MemoryThread(void* arg) {
    NSLog(@"[Cheat] Memory Thread Started!");
    
    // Tunggu sampai Il2Cpp berhasil di-attach
    while (!Il2CppAttach()) {
        sleep(2);
    }
    NSLog(@"[Cheat] Il2Cpp Attached Successfully!");
    
    while(true) {
        void* bmInst = nullptr;
        void* logicBmInst = nullptr;
        
        // Dapatkan static field Instance dari BattleManager
        Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "BattleManager", "Instance", &bmInst);
        
        // Dapatkan static field Instance dari LogicBattleManager
        Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "LogicBattleManager", "Instance", &logicBmInst);
        
        if (bmInst && logicBmInst) {
            // Karena pointer ini adalah objek Il2Cpp langsung, kita casting ke uintptr_t
            g_Battle.Update((uintptr_t)bmInst, (uintptr_t)logicBmInst);
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
