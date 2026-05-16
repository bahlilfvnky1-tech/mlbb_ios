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
        [self setupMetal];
        [self setupImGui];
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
    
    // Agar background transparan (overlay)
    self.mtkView.clearColor = MTLClearColorMake(0, 0, 0, 0);
    [self addSubview:self.mtkView];
}

- (void)setupImGui {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    ImGui::StyleColorsDark();
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
    ImGuiIO& io = ImGui::GetIO();
    
    if (bShowMenu) {
        return [super hitTest:point withEvent:event];
    }
    
    // Jika menu tertutup, cek apakah touch mengenai tombol toggle (CBZ)
    // Posisi tombol toggle di ui_menu.h: x=20, y=DisplaySize.y - 140, w=140, h=140
    CGRect toggleButtonRect = CGRectMake(20, self.bounds.size.height - 140, 140, 140);
    if (CGRectContainsPoint(toggleButtonRect, point)) {
        return [super hitTest:point withEvent:event];
    }
    
    return nil;
}

#pragma mark - MTKViewDelegate

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
}

- (void)drawInMTKView:(MTKView *)view {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = view.bounds.size.width;
    io.DisplaySize.y = view.bounds.size.height;
    
    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];
    MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor;
    
    if (renderPassDescriptor != nil) {
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        
        ImGui_ImplMetal_NewFrame(renderPassDescriptor);
        ImGui::NewFrame();
        
        // Render UI
        ShowMenu();
        RenderRetriDot();
        
        // Render ESP jika data valid
        if (g_Battle.isValid) {
            SyncFeatureToESP();
            RenderESPCore();
        }
        
        ImGui::Render();
        ImDrawData *draw_data = ImGui::GetDrawData();
        ImGui_ImplMetal_RenderDrawData(draw_data, commandBuffer, renderEncoder);
        
        [renderEncoder endEncoding];
        [commandBuffer presentDrawable:view.currentDrawable];
    }
    [commandBuffer commit];
}

@end
