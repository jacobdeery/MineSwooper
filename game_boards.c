#include "game_boards.h"
#include "string.h"

const uint8_t game_boards[4][9][9] = {
    {
        {U_0, U_0, U_1, U_2, U_2, U_2, U_M, U_2, U_1},
        {U_0, U_0, U_1, U_M, U_M, U_2, U_2, U_M, U_1},
        {U_0, U_1, U_2, U_3, U_2, U_1, U_1, U_1, U_1},
        {U_0, U_1, U_M, U_2, U_1, U_1, U_0, U_0, U_0},
        {U_0, U_1, U_1, U_2, U_M, U_1, U_0, U_0, U_0},
        {U_0, U_0, U_0, U_1, U_1, U_1, U_0, U_1, U_1},
        {U_0, U_0, U_0, U_0, U_1, U_1, U_1, U_2, U_M},
        {U_1, U_1, U_1, U_0, U_1, U_M, U_1, U_2, U_M},
        {U_1, U_M, U_1, U_0, U_1, U_1, U_1, U_1, U_1}
    },

    {
        {U_1, U_2, U_M, U_1, U_0, U_0, U_0, U_0, U_0},
        {U_M, U_2, U_1, U_1, U_0, U_0, U_0, U_1, U_1},
        {U_2, U_2, U_0, U_0, U_0, U_1, U_1, U_2, U_M},
        {U_M, U_2, U_1, U_0, U_0, U_1, U_M, U_2, U_1},
        {U_2, U_M, U_1, U_0, U_0, U_1, U_1, U_2, U_1},
        {U_1, U_1, U_1, U_0, U_0, U_0, U_0, U_1, U_M},
        {U_0, U_0, U_0, U_1, U_1, U_2, U_1, U_2, U_1},
        {U_0, U_0, U_0, U_1, U_M, U_3, U_M, U_2, U_0},
        {U_0, U_0, U_0, U_1, U_1, U_3, U_M, U_2, U_0}
    },

    {
        {U_1, U_M, U_2, U_1, U_0, U_0, U_0, U_0, U_0},
        {U_1, U_2, U_M, U_1, U_0, U_0, U_0, U_1, U_1},
        {U_0, U_1, U_1, U_1, U_0, U_0, U_0, U_1, U_M},
        {U_0, U_0, U_0, U_0, U_0, U_1, U_1, U_2, U_1},
        {U_1, U_1, U_0, U_0, U_0, U_2, U_M, U_3, U_1},
        {U_M, U_1, U_0, U_1, U_1, U_3, U_M, U_M, U_2},
        {U_1, U_1, U_1, U_2, U_M, U_2, U_3, U_M, U_2},
        {U_0, U_0, U_1, U_M, U_2, U_1, U_1, U_1, U_1},
        {U_0, U_0, U_1, U_1, U_1, U_0, U_0, U_0, U_0}
    },

    {
        {U_2, U_3, U_M, U_1, U_0, U_0, U_0, U_1, U_1},
        {U_M, U_M, U_2, U_1, U_0, U_0, U_1, U_2, U_M},
        {U_2, U_2, U_2, U_1, U_2, U_2, U_3, U_M, U_2},
        {U_0, U_0, U_1, U_M, U_3, U_M, U_M, U_2, U_1},
        {U_0, U_1, U_2, U_2, U_3, U_M, U_3, U_1, U_0},
        {U_0, U_1, U_M, U_1, U_1, U_1, U_1, U_0, U_0},
        {U_0, U_1, U_1, U_1, U_0, U_0, U_0, U_0, U_0},
        {U_0, U_0, U_0, U_0, U_0, U_0, U_0, U_0, U_0},
        {U_0, U_0, U_0, U_0, U_0, U_0, U_0, U_0, U_0}
    }
};

void Board_GetGameBoardInstance(uint8_t board_num, uint8_t board_ptr[9][9]) {
    memcpy(board_ptr, game_boards[board_num], sizeof(uint8_t)*81);
}

uint8_t Board_RevealTileAt(uint8_t board[9][9], uint8_t x, uint8_t y) {
    if(board[y][x] > U_M && board[y][x] < F_0) {
        return 255; // tile is already revealed
    } else if(board[y][x] > R_M) {
        board[y][x] -= 10;
        return board[y][x];
    } else {
        board[y][x] += 10;
        return board[y][x];
    }
}

uint8_t Board_FlagTileAt(uint8_t board[9][9], uint8_t x, uint8_t y) {
    if(board[y][x] > U_M && board[y][x] < F_0) {
        return 255; // tile is already revealed
    } else if(board[y][x] > R_M) {
        board[y][x] -= 20; // remove existing flag
        return 0;
    } else {
        board[y][x] += 20; // add new flag
        return 1;
    }
}
