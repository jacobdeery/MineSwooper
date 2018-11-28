#ifndef _GAME_BOARDS_H
#define _GAME_BOARDS_H

#include <stdint.h>

#define U_0 0
#define U_1 1
#define U_2 2
#define U_3 3
#define U_4 4
#define U_5 5
#define U_6 6
#define U_7 7
#define U_8 8
#define U_M 9

#define R_0 10
#define R_1 11
#define R_2 12
#define R_3 13
#define R_4 14
#define R_5 15
#define R_6 16
#define R_7 17
#define R_8 18
#define R_M 19

#define F_0 20
#define F_1 21
#define F_2 22
#define F_3 23
#define F_4 24
#define F_5 25
#define F_6 26
#define F_7 27
#define F_8 28
#define F_M 29

void Board_GetGameBoardInstance(uint8_t board_num, uint8_t board_ptr[9][9]);
uint8_t Board_RevealTileAt(uint8_t board[9][9], uint8_t x, uint8_t y);
uint8_t Board_FlagTileAt(uint8_t board[9][9], uint8_t x, uint8_t y);

#endif /* _GAME_BOARDS_H */
