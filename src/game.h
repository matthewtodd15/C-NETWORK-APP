#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <assert.h>
#include "defs.h"
#include "structs.h"

/**
 * Validates ship placement before the start of a game
 * currently set to work with ships of all sizes
 *
 * Valid 7x5 Board (for brevity) view - **not in memory**:
 * 0 0 0 0 0 0 0
 * 2 1 0 3 3 3 0
 * 2 0 2 0 0 0 0
 * 0 0 2 4 4 4 4
 * 1 0 0 0 0 0 0
 *
 * Board in memory:
 * 0 0 0 0 0 0 0
 * 1 1 0 1 1 1 0
 * 1 0 1 0 0 0 0
 * 0 0 1 1 1 1 1
 * 1 0 0 0 0 0 0
 *
 * Each ship is also an integer storing positions on the grid:
 * Ship 1a:
 * 0 0 0 0 0 0 0
 * 0 1 0 0 0 0 0
 * 0 0 0 0 0 0 0
 * 0 0 0 0 0 0 0
 * 0 0 0 0 0 0 0
 *
 * Ship 2:
 * 0 0 0 0 0 0 0
 * 0 0 0 0 0 0 0
 * 0 0 1 0 0 0 0
 * 0 0 1 0 0 0 0
 * 0 0 0 0 0 0 0
 *
 * Ship 4:
 * 0 0 0 0 0 0 0
 * 0 0 0 0 0 0 0
 * 0 0 0 0 0 0 0
 * 0 0 0 1 1 1 1
 * 0 0 0 0 0 0 0
 *
 * This makes it so that when we hit a ship on the board, we can clear the bit
 * we hit in the corresponding ship and check it against 0. if 0, we know we
 * sunk a ship.
 *
 * To validate the board, we can AND them and check against 0 (all combinations)
 * if (ship4 & ship1a != 0) {
 *   Invalid position for ship1a
 * }
 *
 * We'll also need to make sure you cant wrap your ship around the board:
 *
 * Ship 4 (Invalid):
 * 0 0 0 0 0 0 0
 * 0 0 0 0 0 0 0
 * 0 0 0 0 0 0 0
 * 0 0 0 0 1 1 1
 * 1 0 0 0 0 0 0
 *
 * And that you make longer ships the correct length, like ship 4 should have 4
 * 1's together
 *
 * @param int array with ship locations. Each integer should be the same
 * precision as the board (64 bits for a 8x8 board)
 */
void validate_board(uint64_t *ships, game_data *gameData);

/**
 * Checks each player_board against their opponent's hit_board to
 * see if the game has been won (all ships are sunk)
 * 
 * If the game is won, it sets the winner to 0 or 1 and returns TRUE.
 * If the game is not won, returns FALSE and winner remains -1 (default)
 *
 * @param game_data struct at any point in the game
 */
bool check_game_over(game_data *gameData);

#endif
