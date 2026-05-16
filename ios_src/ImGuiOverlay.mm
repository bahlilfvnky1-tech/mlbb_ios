#import "ImGuiOverlay.h"
#import "imgui/imgui.h"
#import "imgui/backends/imgui_impl_metal.h"

// Include ESP
#include "esp/ui_menu.h"
#include "esp/esp_core.h"
#include "esp/esp_player.h"

@interface ImGuiOverlay () <MTKViewDelegate>
@property (nonatomic, strong) MTKView *mtkView;
@property (nonatomic, strong) id <MTLDevice> device;
@property (nonatomic, strong) id <MTLCommandQueue> commandQueue;
@property (nonatomic, strong) UIButton *toggleButton;
@end

@implementation ImGuiOverlay

+ (instancetype)sharedOverlay {
    static ImGuiOverlay *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        UIWindow *window = [UIApplication sharedApplication].keyWindow;
        sharedInstance = [[ImGuiOverlay alloc] initWithFrame:window.bounds];
        [window addSubview:sharedInstance];
    });
    return sharedInstance;
}

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        self.backgroundColor = [UIColor clearColor];
        self.userInteractionEnabled = YES; // Agar bisa terima touch
        self.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        
        [self setupMetal];
        [self setupImGui];
        [self setupToggleButton];
    }
    return self;
}

- (void)setupMetal {
    self.device = MTLCreateSystemDefaultDevice();
    self.commandQueue = [self.device newCommandQueue];
    
    self.mtkView = [[MTKView alloc] initWithFrame:self.bounds device:self.device];
    self.mtkView.backgroundColor = [UIColor clearColor];
    self.mtkView.delegate = self;
    self.mtkView.framebufferOnly = NO;
    self.mtkView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    
    // Agar background transparan (overlay)
    self.mtkView.clearColor = MTLClearColorMake(0, 0, 0, 0);
    [self addSubview:self.mtkView];
}

- (void)setupToggleButton {
    // Buat tombol native iOS agar 100% bisa diklik tanpa masalah hitTest
    self.toggleButton = [UIButton buttonWithType:UIButtonTypeCustom];
    self.toggleButton.frame = CGRectMake(20, self.bounds.size.height - 80, 50, 50);
    self.toggleButton.backgroundColor = [UIColor colorWithRed:0 green:0 blue:0 alpha:0.6];
    self.toggleButton.layer.cornerRadius = 25;
    self.toggleButton.layer.borderWidth = 1.5;
    self.toggleButton.layer.borderColor = [UIColor whiteColor].CGColor;
    [self.toggleButton setTitle:@"CBZ" forState:UIControlStateNormal];
    [self.toggleButton setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
    self.toggleButton.titleLabel.font = [UIFont boldSystemFontOfSize:14];
    
    [self.toggleButton addTarget:self action:@selector(onToggleClicked) forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:self.toggleButton];
}

- (void)onToggleClicked {
    bShowMenu = !bShowMenu;
    self.toggleButton.hidden = bShowMenu; // Sembunyikan tombol native jika menu ImGui terbuka
}

- (void)layoutSubviews {
    [super layoutSubviews];
    self.mtkView.frame = self.bounds;
    self.toggleButton.frame = CGRectMake(20, self.bounds.size.height - 80, 50, 50);
}

- (void)setupImGui {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    ImGui::StyleColorsDark();
    
    // Kembalikan skala ke normal (1.0x) agar menu tidak 'gede banget'
    // DisplayFramebufferScale sudah cukup untuk membuat resolusinya tajam di Retina
    ImGui::GetStyle().ScaleAllSizes(1.0f);
    io.FontGlobalScale = 1.0f;
    
    ImGui_ImplMetal_Init(self.device);
    
    // Konfigurasi IO Display
    io.DisplaySize.x = self.bounds.size.width;
    io.DisplaySize.y = self.bounds.size.height;
}

- (void)updateIOWithTouchEvent:(UIEvent *)event {
    UITouch *anyTouch = event.allTouches.anyObject;
    CGPoint location = [anyTouch locationInView:self];
    
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(location.x, location.y);
    
    if (anyTouch.phase == UITouchPhaseBegan) {
        io.AddMouseButtonEvent(0, true);
    } else if (anyTouch.phase == UITouchPhaseEnded || anyTouch.phase == UITouchPhaseCancelled) {
        io.AddMouseButtonEvent(0, false);
    }
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [self updateIOWithTouchEvent:event];
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [self updateIOWithTouchEvent:event];
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [self updateIOWithTouchEvent:event];
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [self updateIOWithTouchEvent:event];
}

// Meneruskan touch ke game jika menu ImGui tidak disentuh
- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event {
    if (bShowMenu) {
        // Jika menu terbuka, cek apakah touch mengenai window ImGui
        // ImGuiIO& io = ImGui::GetIO();
        // io.WantCaptureMouse akan true jika kita menyentuh window ImGui
        return [super hitTest:point withEvent:event];
    }
    
    // Jika menu tertutup, hanya tombol native yang bisa disentuh
    if (!self.toggleButton.hidden && CGRectContainsPoint(self.toggleButton.frame, point)) {
        return self.toggleButton;
    }
    
    return nil; // Sentuhan diteruskan ke game
}

#pragma mark - MTKViewDelegate

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
}

- (void)drawInMTKView:(MTKView *)view {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = view.bounds.size.width;
    io.DisplaySize.y = view.bounds.size.height;
    
    // SANGAT PENTING: Set FramebufferScale agar sentuhan akurat dan menu tidak terpotong (clipping) di layar Retina
    io.DisplayFramebufferScale = ImVec2(view.contentScaleFactor, view.contentScaleFactor);
    
    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];
    MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor;
    
    if (renderPassDescriptor != nil) {
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        
        ImGui_ImplMetal_NewFrame(renderPassDescriptor);
        ImGui::NewFrame();
        
        // Render UI
        ShowMenu();
        RenderRetriDot();
        
        // Sinkronisasi status tombol toggle native setiap frame
        // Panggil Menu UI
        // ShowMenu() sudah dipanggil di atas
        
        // Sinkronisasi data entitas dari thread pemindai memori
        g_Battle.SwapBuffers();
        
        // DEBUG TEXT DI LAYAR
        char debugStr[768];
        snprintf(debugStr, sizeof(debugStr), 
                 "DEBUG:\n"
                 "BattleState: %d\n"
                 "bmInst: %p\n"
                 "logicBmInst: %p\n"
                 "dicPlayerOff: 0x%zx\n"
                 "dicPlayerPtr: 0x%lx\n"
                 "entries: 0x%lx (maxLen: %d)\n"
                 "1stEntity: 0x%lx\n"
                 "Heroes: %zu / count: %d\n"
                 "Monsters: %zu\n"
                 "LocalCamp: %d | Camera: %p", 
                 g_Battle.dbg_battleState, (void*)g_Battle.dbg_bmInst, (void*)g_Battle.dbg_logicBmInst,
                 g_Battle.dbg_dicPlayerOff, g_Battle.dbg_dicPlayerPtr,
                 g_Battle.dbg_entriesPtr, g_Battle.dbg_entriesMaxLen,
                 g_Battle.dbg_firstEntityPtr,
                 g_Battle.heroes_render.size(), g_Battle.dbg_dicPlayerCount,
                 g_Battle.monsters_render.size(),
                 g_Battle.localCamp, GetCameraMain());
        char extraStr[256] = "";
        if (!g_Battle.heroes_render.empty()) {
            for (auto& e : g_Battle.heroes_render) {
                if (e.camp != g_Battle.localCamp && !e.isSelf) {
                    snprintf(extraStr, sizeof(extraStr), "\nEnemyPos: %.2f, %.2f, %.2f", e.pos.x, e.pos.y, e.pos.z);
                    break;
                }
            }
        }
        
        std::string finalDebugStr = std::string(debugStr) + extraStr;
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(50, 50), IM_COL32(0, 255, 0, 255), finalDebugStr.c_str());
        // Panggil ESP Core (hanya jika fitur aktif)
        if (g_Battle.isValid) {
            SyncFeatureToESP();
            RenderESPCore();
        }
        
        // Sinkronisasi status tombol toggle native setiap frame
        self.toggleButton.hidden = bShowMenu;
        
        ImGui::Render();
        ImDrawData *draw_data = ImGui::GetDrawData();
        ImGui_ImplMetal_RenderDrawData(draw_data, commandBuffer, renderEncoder);
        
        [renderEncoder endEncoding];
        [commandBuffer presentDrawable:view.currentDrawable];
    }
    [commandBuffer commit];
}

@end
