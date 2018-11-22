#ifndef _DISPLAY_MANAGER_H
#define _DISPLAY_MANAGER_H

#include <stdint.h>

void Display_Init(void);

void Display_SetBoardOffsetX(uint16_t offset);
void Display_SetBoardOffsetY(uint16_t offset);

void Display_ShowMainMenu(void);
void Display_ShowGameBoard(void);
void Display_ShowVictory(void);
void Display_ShowGameOver(void);

void Display_UpdateTimeLimit(uint32_t seconds);
void Display_UpdateTimeRemaining(uint32_t seconds);

void Display_DrawSpriteSheet(void);
void Display_DrawCell(uint8_t x, uint8_t y, uint8_t type);
void Display_DrawCellNum(uint8_t x, uint8_t y, uint8_t num);

void Display_PlaceCursorAt(uint8_t x, uint8_t y);

#endif /* _DISPLAY_MANAGER_H */
