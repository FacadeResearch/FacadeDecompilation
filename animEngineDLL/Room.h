#pragma once
#include <cstdint>

class Sprite;

Sprite* CreatePanel(int spriteId, uint32_t nameId, int param3, int objectType, uint32_t param5,
                    uint32_t param6, uint32_t param7, uint32_t param8, uint32_t param9, int param10,
                    int param11, uint32_t color1, uint32_t texture, uint32_t color2, uint32_t posX,
                    uint32_t posY, uint32_t posZ, int param16, int param17);

void CreateCouchSprite(int spriteId, uint32_t nameId);
void CreateCabinetSprite(int spriteId, uint32_t nameId);
void CreateSideTableSprite(int spriteId, uint32_t nameId);
void CreateWorkTableSprite(int spriteId, uint32_t nameId);
void CreateBarSprite(int spriteId, uint32_t nameId);
void CreateLuxTableSprite(int spriteId, uint32_t nameId);

int GetFullDrinkLevel(bool isTrip);

void CreateRoom();
