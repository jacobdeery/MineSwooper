#ifndef _DISPLAY_MANAGER_H
#define _DISPLAY_MANAGER_H

#include <stdint.h>

void Display_Init(void);
void Display_Clear(void);

void Display_SetBoardOffsetX(uint16_t offset);
void Display_SetBoardOffsetY(uint16_t offset);

void Display_ShowMainMenu(void);
void Display_ShowGameBoard(void);
void Display_ShowVictory(void);
void Display_ShowGameOver(uint8_t exploded);

void Display_UpdateTimeLimit(uint8_t seconds);
void Display_UpdateTimeRemaining(uint8_t seconds);
void Display_UpdateMinesRemaining(int8_t mines);

void Display_DrawSpriteSheet(void);
void Display_DrawCell(uint8_t x, uint8_t y, uint8_t type);
void Display_DrawCellNum(uint8_t x, uint8_t y, uint8_t num);
void Display_DrawCursor(uint8_t x, uint8_t y);

#endif /* _DISPLAY_MANAGER_H */
