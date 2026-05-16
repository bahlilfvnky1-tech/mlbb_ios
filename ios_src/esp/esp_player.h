#pragma once
#include "imgui.h"
#include "config.h"
#include "game_math.h"
#include "esp_core.h"
#include "esp_minimap.h"
#include <string>

inline void DrawMonsterESP(ImDrawList* draw, void* camera, float screenW, float screenH) {
    float CurrentFOVScale = 1.0f; 

    for (auto& e : g_Battle.monsters_render) {
        if (e.isDead || e.hp <= 0) continue;

        Vec2 rootPosW2S;
        if(!UnityWorldToScreen(camera, e.pos, rootPosW2S, screenH)) continue;
        
        // --- 3D PERSPECTIVE HEIGHT ---
        Vec3 headPos3D = e.pos;
        headPos3D.y += 1.0f; // Tinggi rata-rata monster lebih pendek
        Vec2 headPosW2S;
        ImVec2 headPosVec2;
        if(UnityWorldToScreen(camera, headPos3D, headPosW2S, screenH)) {
            headPosVec2 = ImVec2(headPosW2S.x, headPosW2S.y);
        } else {
            headPosVec2 = ImVec2(rootPosW2S.x, rootPosW2S.y - 40.0f);
        }
        
        ImVec2 rootPosVec2(rootPosW2S.x, rootPosW2S.y);
        
        // ================== MONSTER ROUND / ICON ==================
        float circleRadius = 10.0f / CurrentFOVScale;
        draw->AddCircleFilled(ImVec2(rootPosVec2.x, headPosVec2.y), circleRadius, IM_COL32(160, 32, 240, 200));
        draw->AddCircle(ImVec2(rootPosVec2.x, headPosVec2.y), circleRadius, IM_COL32(255, 255, 255, 255), 0, 1.5f);
        
        // ================== MONSTER NAME ==================
        if (e.name.length() > 0) {
            float fontSize = 16.0f / CurrentFOVScale;
            auto textSize = ImGui::CalcTextSize(e.name.c_str());
            float scaledTextWidth = (textSize.x * fontSize) / ImGui::GetFontSize();
            
            ImVec2 textPos = {rootPosVec2.x - (scaledTextWidth / 2), headPosVec2.y - 15.0f};
            draw->AddText(NULL, fontSize, ImVec2(textPos.x + 1.0f, textPos.y + 1.0f), IM_COL32(0, 0, 0, 240), e.name.c_str());
            draw->AddText(NULL, fontSize, textPos, IM_COL32(255, 255, 255, 255), e.name.c_str());
        }

        // ================== MONSTER HEALTH ==================
        float healthPercent = (float)e.hp / (float)e.hpMax;
        ImU32 healthColor = IM_COL32(50, 255, 50, 255);
        if (healthPercent <= 0.3f) healthColor = IM_COL32(255, 50, 50, 255);
        else if (healthPercent <= 0.5f) healthColor = IM_COL32(255, 200, 50, 255);
        
        float healthBarWidth = 40.0f / CurrentFOVScale;
        float healthBarHeight = 6.0f / CurrentFOVScale;
        float healthBarX = rootPosVec2.x - (healthBarWidth / 2);
        float baseY = rootPosVec2.y - 15 / CurrentFOVScale;
        
        ImVec2 bgStart = {healthBarX, baseY};
        ImVec2 bgEnd = {healthBarX + healthBarWidth, baseY + healthBarHeight};
        ImVec2 healthEnd = {healthBarX + (healthBarWidth * healthPercent), baseY + healthBarHeight};
        
        draw->AddRectFilled(bgStart, bgEnd, IM_COL32(0, 0, 0, 200), 2);
        draw->AddRectFilled(bgStart, healthEnd, healthColor, 2);
        draw->AddRect(bgStart, bgEnd, IM_COL32(255, 255, 255, 150), 2, 0, 1.0f);
    }
}

inline void DrawPlayerESP(ImDrawList* draw, void* camera, float screenW, float screenH, bool hasSelf, const ImVec2& selfPosVec2) {
    float CurrentFOVScale = 1.0f; 

    for (auto& e : g_Battle.heroes_render) {
        if (e.isSelf || e.isDead || e.hp <= 0) continue;
        if (e.camp == g_Battle.localCamp) continue; // Only Enemy

        DrawMinimapIcon(draw, e.entityId, e.hp, e.hpMax, e.pos, g_Battle.localCamp);

        Vec2 rootPosW2S;
        // Gunakan UnityWorldToScreen yang memanggil Camera.WorldToScreenPoint via Il2Cpp
        if(!UnityWorldToScreen(camera, e.pos, rootPosW2S, screenH)) continue;
        
        ImVec2 rootPosVec2(rootPosW2S.x, rootPosW2S.y);
        
        // --- 3D PERSPECTIVE HEIGHT (Lebih Akurat) ---
        Vec3 headPos3D = e.pos;
        headPos3D.y += 1.8f; // Tinggi rata-rata hero di Unity
        Vec2 headPosW2S;
        ImVec2 HeadPosVec2;
        if(UnityWorldToScreen(camera, headPos3D, headPosW2S, screenH)) {
            HeadPosVec2 = ImVec2(headPosW2S.x, headPosW2S.y);
        } else {
            // Fallback
            float dynamicOffset = GetDynamicOffset(screenH, CurrentFOVScale);
            HeadPosVec2 = ImVec2(rootPosVec2.x, rootPosVec2.y - dynamicOffset);
        }
        
        float baseY = rootPosVec2.y + (55.0f / CurrentFOVScale);

        // ================== LINE ==================
        if (g_ESPCfg.ESPLine && hasSelf) {
            ImVec2 startPoint = ImVec2(rootPosVec2.x, rootPosVec2.y + 10 / CurrentFOVScale);
            ImVec2 endPoint = selfPosVec2;
            
            int segmentCount = 20;
            for (int i = 0; i < segmentCount; i++) {
                float t1 = (float)i / segmentCount;
                float t2 = (float)(i + 1) / segmentCount;
                
                ImVec2 p1 = ImVec2(startPoint.x + (endPoint.x - startPoint.x) * t1, startPoint.y + (endPoint.y - startPoint.y) * t1);
                ImVec2 p2 = ImVec2(startPoint.x + (endPoint.x - startPoint.x) * t2, startPoint.y + (endPoint.y - startPoint.y) * t2);
                
                int alpha1 = 220 - (int)(t1 * 220);
                int alpha2 = 220 - (int)(t2 * 220);
                alpha1 = (alpha1 < 0) ? 0 : alpha1;
                alpha2 = (alpha2 < 0) ? 0 : alpha2;
                draw->AddLine(p1, p2, IM_COL32(255, 255, 255, alpha2), 1.5f);
            }
        }

        // ================== BOX ==================
        if (g_ESPCfg.ESPBox) {
            float boxHeight = fabs(HeadPosVec2.y - rootPosVec2.y) * 1.15f;
            float boxWidth = boxHeight * 0.8f;
            
            ImVec2 vStart = {HeadPosVec2.x - (boxWidth / 2), HeadPosVec2.y};
            ImVec2 vEnd = {vStart.x + boxWidth, vStart.y + boxHeight};
            
            draw->AddRectFilled(vStart, vEnd, IM_COL32(0, 0, 0, 50), g_ESPCfg.RoundBox / 2);
            draw->AddRectFilled(vStart, vEnd, IM_COL32(100, 200, 255, 30), g_ESPCfg.RoundBox / 2);
            draw->AddRect(vStart, vEnd, IM_COL32(100, 200, 255, 255), g_ESPCfg.RoundBox, 0, 3.0f);
        }

        // ================== ROUND ==================
        if (g_ESPCfg.ESPRound) {
            float circleRadius = 10.0f / CurrentFOVScale;
            draw->AddCircleFilled(ImVec2(rootPosVec2.x, rootPosVec2.y + 10 / CurrentFOVScale), circleRadius, IM_COL32(100, 200, 255, 255));
            draw->AddCircle(ImVec2(rootPosVec2.x, rootPosVec2.y + 10 / CurrentFOVScale), circleRadius, IM_COL32(255, 255, 255, 200), 0, 2.0f);
        }

        // ================== HEALTH BAR ==================
        if (g_ESPCfg.ESPHealth) {
            float baseHealthBarWidth = 160.0f;
            float healthBarWidth = baseHealthBarWidth / CurrentFOVScale;
            float healthBarHeight = 24.0f / CurrentFOVScale;
            
            float healthBarX = rootPosVec2.x - (healthBarWidth / 2);
            ImVec2 bgStart = {healthBarX, baseY};
            ImVec2 bgEnd = {healthBarX + healthBarWidth, baseY + healthBarHeight};
            
            float healthPercent = (float)e.hp / (float)e.hpMax;
            ImVec2 healthEnd = {healthBarX + (healthBarWidth * healthPercent), baseY + healthBarHeight};
            
            ImU32 healthColor = GetHealthColor(e.hp, e.hpMax);
            
            draw->AddRectFilled(bgStart, bgEnd, IM_COL32(0, 0, 0, 200), 8);
            draw->AddRectFilled(bgStart, healthEnd, healthColor, 8);
            draw->AddRect(bgStart, bgEnd, IM_COL32(255, 255, 255, 230), 8, 0, 2.0f);
            
            std::string hpText = std::to_string(e.hp) + " / " + std::to_string(e.hpMax);
            float baseHpFontSize = 20.0f;
            float hpFontSize = baseHpFontSize / CurrentFOVScale;
            
            auto hpTextSize = ImGui::CalcTextSize(hpText.c_str());
            float scaledHpTextWidth = (hpTextSize.x * hpFontSize) / ImGui::GetFontSize();
            ImVec2 hpTextPos = {rootPosVec2.x - (scaledHpTextWidth / 2), baseY + (4.0f / CurrentFOVScale)};
            
            draw->AddText(NULL, hpFontSize, ImVec2(hpTextPos.x + 1, hpTextPos.y + 1), IM_COL32(0, 0, 0, 255), hpText.c_str());
            draw->AddText(NULL, hpFontSize, hpTextPos, IM_COL32(255, 255, 255, 255), hpText.c_str());
            
            baseY += healthBarHeight + (8.0f / CurrentFOVScale);
        }

        // ================== NAME ==================
        if (g_ESPCfg.ESPName) {
            std::string heroName = e.name;
            float baseNameFontSize = screenH / 30.0f;
            float nameFontSize = baseNameFontSize / CurrentFOVScale;
            
            auto textSize = ImGui::CalcTextSize(heroName.c_str());
            float scaledTextWidth = (textSize.x * nameFontSize) / ImGui::GetFontSize();
            ImVec2 textPos = {rootPosVec2.x - (scaledTextWidth / 2), baseY};
            
            draw->AddText(NULL, nameFontSize, ImVec2(textPos.x + 2.0f, textPos.y + 2.0f), IM_COL32(0, 0, 0, 250), heroName.c_str());
            draw->AddText(NULL, nameFontSize, textPos, IM_COL32(220, 180, 255, 255), heroName.c_str());
            baseY += (textSize.y * nameFontSize / ImGui::GetFontSize()) + 5.0f;
        }

        // ================== HERO ICON (OVERLAY) ==================
        if (g_ESPCfg.ESPHero) {
            GLuint tex = GetHeroIcon(e.entityId);
            if (tex != 0) {
                float iconSize = 48.0f / CurrentFOVScale;
                float halfSize = iconSize / 2.0f;
                ImVec2 iconMin = ImVec2(rootPosVec2.x - halfSize, baseY);
                ImVec2 iconMax = ImVec2(rootPosVec2.x + halfSize, baseY + iconSize);
                
                draw->AddImageRounded((void*)(intptr_t)tex, iconMin, iconMax, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255), 6.0f);
                draw->AddRect(iconMin, iconMax, IM_COL32(255, 255, 255, 180), 6.0f, 0, 1.5f);
                
                baseY += iconSize + (8.0f / CurrentFOVScale);
            }
        }

        // ================== COOLDOWNS ==================
        if (g_ESPCfg.ESPSkillCD || g_ESPCfg.ESPSpellCD) {
            float baseCdBoxSize = 48.0f;
            float baseCdSpacing = 56.0f;
            
            float cdBoxSize = baseCdBoxSize / CurrentFOVScale;
            float cdSpacing = baseCdSpacing / CurrentFOVScale;
            
            int numSkillsToDraw = e.n_skills > 0 ? e.n_skills : 3;
            if (numSkillsToDraw > 4) numSkillsToDraw = 4;
            
            int totalSkills = (g_ESPCfg.ESPSkillCD ? numSkillsToDraw : 0) + (g_ESPCfg.ESPSpellCD ? 1 : 0);
            float totalWidth = (totalSkills - 1) * cdSpacing;
            float startX = rootPosVec2.x - (totalWidth / 2);
            
            int skillIndex = 0;
            baseY += (30.0f / CurrentFOVScale);
            
            auto DrawCooldownBox = [&](ImVec2 basePos, int type, int cd_ms) {
                float boxSize = baseCdBoxSize / CurrentFOVScale;
                float fontSize = (baseCdBoxSize * 0.75f) / CurrentFOVScale;
                
                ImVec2 boxStart = ImVec2(basePos.x - boxSize/2, basePos.y - boxSize/2);
                ImVec2 boxEnd = ImVec2(basePos.x + boxSize/2, basePos.y + boxSize/2);
                
                int cooldown = (cd_ms > 0) ? ((cd_ms + 999) / 1000) : 0;
                
                ImU32 color;
                if (type == 0) {
                    color = IM_COL32(100, 200, 255, 255);
                } else if (type == 4) {
                    color = IM_COL32(255, 120, 120, 255);
                } else {
                    color = IM_COL32(255, 220, 100, 255);
                }
                
                if (cooldown > 0) {
                    draw->AddRectFilled(boxStart, boxEnd, IM_COL32(0, 0, 0, 200), boxSize * 0.15f);
                    draw->AddRect(boxStart, boxEnd, IM_COL32(255, 255, 255, 100), boxSize * 0.15f, 0, 2.0f);
                    
                    std::string strCoolDown = std::to_string(cooldown);
                    auto textSize = ImGui::CalcTextSize(strCoolDown.c_str());
                    ImVec2 textPos = ImVec2(basePos.x - (textSize.x * fontSize / ImGui::GetFontSize()) / 2, 
                                            basePos.y - (textSize.y * fontSize / ImGui::GetFontSize()) / 2);
                    
                    draw->AddText(NULL, fontSize, ImVec2(textPos.x + 1.0f, textPos.y + 1.0f), 
                                 IM_COL32(0, 0, 0, 255), strCoolDown.c_str());
                    draw->AddText(NULL, fontSize, textPos, color, strCoolDown.c_str());
                } else {
                    draw->AddRectFilled(boxStart, boxEnd, IM_COL32(color >> 16 & 0xFF, color >> 8 & 0xFF, color & 0xFF, 100), boxSize * 0.15f);
                    draw->AddRect(boxStart, boxEnd, color, boxSize * 0.15f, 0, 2.5f);
                    draw->AddCircleFilled(basePos, boxSize * 0.2f, color);
                }
            };
            
            if (g_ESPCfg.ESPSkillCD) {
                for (int i = 0; i < numSkillsToDraw; i++) {
                    DrawCooldownBox(ImVec2(startX + (skillIndex * cdSpacing), baseY), 1, e.skill_cd[i]);
                    skillIndex++;
                }
            }
            if (g_ESPCfg.ESPSpellCD) {
                DrawCooldownBox(ImVec2(startX + (skillIndex * cdSpacing), baseY), 0, e.spell_cd);
            }
        }
    }
}
