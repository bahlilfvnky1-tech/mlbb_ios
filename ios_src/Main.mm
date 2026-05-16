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
    
    // SANGAT PENTING: Tunggu game sampai benar-benar masuk ke menu/loading screen
    // Jika kita memanggil Il2CppGetStaticFieldValue terlalu cepat, Unity akan mencoba
    // menginisialisasi BattleManager sebelum engine siap, yang menyebabkan EXC_BAD_ACCESS (Crash).
    NSLog(@"[Cheat] Waiting 15 seconds for Unity engine to boot...");
    sleep(15);
    
    while(true) {
        void* bmInst = nullptr;
        void* logicBmInst = nullptr;
        
        // Dapatkan static field Instance dari BattleManager (Bukan GameLogic, persis Code Breaker)
        Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "BattleManager", "Instance", &bmInst);
        
        // Dapatkan static field Instance dari LogicBattleManager
        Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "LogicBattleManager", "Instance", &logicBmInst);
        
        int battleState = 0;
        if (logicBmInst) {
            static void* getBattleStateMethod = nullptr;
            if (!getBattleStateMethod) {
                getBattleStateMethod = Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "LogicBattleManager", "GetBattleState", 0);
            }
            if (getBattleStateMethod) {
                // Di ARM64 iOS, calling convention default adalah yang digunakan, cukup cast ke int(*)(void*)
                battleState = reinterpret_cast<int(*)(void*)>(getBattleStateMethod)(logicBmInst);
            }
        }
        
        // Kembalikan ke logika Code Breaker asli:
        // Harus mengecek battleState == 2 || battleState == 3 (berarti sedang dalam match/room)
        if (bmInst && logicBmInst && (battleState == 2 || battleState == 3)) {
            // Karena pointer ini adalah objek Il2Cpp langsung, kita casting ke uintptr_t
            g_Battle.Update((uintptr_t)bmInst, (uintptr_t)logicBmInst);
        } else {
            g_Battle.isValid = false; // Disable ESP di Lobby
        }
        
        usleep(30000); // 30ms sleep (~33 fps ESP update rate)
    }
    
    return NULL;
}

void SetupUI() {
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        UIWindow *window = [UIApplication sharedApplication].keyWindow;
        if (!window) {
            for (UIWindow *w in [UIApplication sharedApplication].windows) {
                if (w.isKeyWindow) { window = w; break; }
            }
        }
        
        if (window) {
            NSLog(@"[Cheat] UIWindow found. Initializing UI Overlay...");
            [ImGuiOverlay sharedOverlay];
        } else {
            NSLog(@"[Cheat] UIWindow not ready yet. Retrying...");
            SetupUI();
        }
    });
}

// Atribut constructor menjamin fungsi ini dipanggil otomatis 
// sesaat setelah libmlbb_cheat.dylib berhasil di-load oleh iOS
__attribute__((constructor))
void InitCheat() {
    NSLog(@"[Cheat] Dylib Injected Successfully!");
    
    // Setup UI di Main Thread dengan retry mechanism
    SetupUI();

    // Jalankan Memory Scanner di background thread
    pthread_t ptid;
    pthread_create(&ptid, NULL, MemoryThread, NULL);
}
