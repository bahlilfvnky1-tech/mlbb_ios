#pragma once
#include <string>
#include "imgui.h"

// Variables Dummy
extern bool bShowMenu;
extern bool bFlagAutoResize;
extern float window_scale;
extern bool bFullChecked;
extern std::string tittle;
extern std::string toastsaveload;
extern int screenWidth;
extern int screenHeight;

struct FeatureState {
    bool MinimapIcon = true;
    bool HideLine = false;
    bool ESPLine = false;
    bool ESPBox = true;
    bool ESPName = true;
    bool ESPHero = true;
    bool ESPRound = false;
    bool ESPHealth = true;
    bool ESPAlertIcon = true;
    bool ESPAlert = true;
    bool ESPMinion = false;
    bool ESPMAlert = false;
    bool ESPMRound = false;
    bool ESPMHealth = false;
    bool ESPMBox = false;
    bool ESPMName = false;
    bool ESPSkillCD = true;
    bool ESPSpellCD = true;
    float ESPOffsetX = 0.0f;
    float ESPOffsetY = -30.0f;
    float ESPScale = 1.0f;
};
extern FeatureState Feature;

struct EspConfig {
    bool ESPBox;
    bool ESPLine;
    bool ESPRound;
    bool ESPName;
    bool ESPHero;
    bool ESPHealth;
    bool ESPAlert;
    bool ESPAlertIcon;
    bool ESPHealthVertical;
    float RoundBox = 8.0f;
    int AlertSz = 64;
    int IconSZ = 64;
    
    bool ESPSkillCD;
    bool ESPSpellCD;
    
    bool ESPMRound;
    bool ESPMHealth;
    bool ESPMinion;
    bool ESPMName;
    bool ESPMBox;
    bool ESPMAlert;
    
    // Minimap Radar
    bool MinimapESP;
    bool MinimapHideBackground;
    float MinimapPosX = 86.25f;
    float MinimapPosY = 0.0f;
    int MinimapSize = 274;
    int MinimapIconSize = 32;
    
    // Screen Offsets untuk ESP (Kompensasi Safe Area / Notch iOS)
    float ScreenOffsetX = 0.0f;
    float ScreenOffsetY = -30.0f; // -30.0f karena secara default m_vCachePosition adalah posisi kaki
    float ScreenScale = 1.0f; // Skala (jika layar iPhone menggunakan Retina scale yang beda)
};
extern EspConfig g_ESPCfg;

// Variabel Auto Retri
extern bool AutoRetriEnabled;
extern bool RetriLord;
extern bool RetriTurtle;
extern bool RetriCrab;
extern bool RetriBuff;
extern bool RetriLitho;
extern ImVec2 RetriPos;

// UI & Menu Variables
extern float SetFieldOfView;
extern bool AutoLoadSettings;

// Posisi & Ukuran
struct Vector2D { float x, y; };
extern Vector2D StartPos;
extern int MapSize;
extern int ICSize;
extern int ICHealthThin;
extern bool isTeam;
extern bool isEnemy;
extern int selectedOption;
