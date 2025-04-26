#include "structs.h"
extern server_ctx server;

#include "game.h"
#include "structs.h"
#include <stdio.h>

const int SHIP_LENGTHS[] = {1, 2, 2, 3, 3, 4};

/**
 * helper function for parsing a board string from client into
 * integers for our server-side validation
 *
 * incoming string should be 64 bytes long (8x8 board), with
 * the only valid characters being 0 to 4 (for ship types)
 *
 * Validates ship placement before the start of a game
 *
 * Valid 7x5 Board (for brevity) view - **in memory as char array**:
 * 0 0 0 0 0 0 0
 * 2 1 0 3 3 3 0
 * 2 0 2 0 0 0 0
 * 0 0 2 4 4 4 4
 * 1 0 0 0 0 0 0
 *
 * NOTE: this format naturally disallows overlap on a single coordinate but not
 * ship overlaps
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
 * @param char array from client-side GAME_MSG_BOARD_SETUP message
 * @param uint64_t array with ship locations.
 * precision as the board (64 bits for a 8x8 board)
 */
int parse_validate_board(unsigned char *board_str, uint64_t ships[NUM_SHIPS],
                         uint64_t *board)
{
  // board_str must be 64 bytes because 8x8 board size
  for (int i = 0; i < 64; i++) {
    if (board_str[i] < 0 || board_str[i] > NUM_SHIPS) {
      printf("Invalid board value %x at %d\n", board_str[i], i);
      return 1;
    }
    else if (board_str[i] > 0) {
      // flip bit of current position in the board and associated ship
      ships[board_str[i] - 1] |= (1 << i);
      *board |= (1 << i);
    }
  }

  // check for wrapping using maskes on the edges (1 for each edge = 7 total)
  uint64_t edge = 3;
  uint64_t edge_masks[7] = {edge << 7,  edge << 15, edge << 23, edge << 31,
                            edge << 39, edge << 47, edge << 55};

  for (int i = 0; i < NUM_SHIPS; i++) {
    for (int j = 0; j < 7; j++) {
      if ((ships[i] & edge_masks[j]) == edge_masks[j]) {
        printf("Ship wrapping detected for ship %d, edge %lx\n", i,
               edge_masks[j]);
        return 2;
      }
    }
  }

  /*
   * check for gaps in the ships (all bits should be consecutive)
   * loop through ship lengths, count consecutive bits to make
   * sure there are no gaps and correct length
   */
  int num_consecutive_bits;
  int prev_bit_on;
  for (int i = 0; i < NUM_SHIPS; i++) {
    num_consecutive_bits = 0;
    prev_bit_on = 0;
    for (int j = 0; j < 64; j++) {
      if ((ships[i] & (1 << j))) {
        if (prev_bit_on) {
          num_consecutive_bits++;
        }
        else {
          prev_bit_on = 1;
          num_consecutive_bits = 1;
        }
      }
    }
    if (num_consecutive_bits != SHIP_LENGTHS[i]) {
      printf("Invalid format or length for ship %d\n", i);
      return 3;
    }
  }

  return 0;
}

/**
 * Checks each player_board against their opponent's hit_board to
 * see if the game has been won (all ships are sunk)
 *
 * If the game is won, it sets the winner to 0 or 1 and returns TRUE.
 * If the game is not won, returns FALSE and winner remains -1 (default)
 *
 * @param game_data struct at any point in the game
 */
bool check_game_over(game_data *gd)
{
  // if player 1 has hit every position on player 2's board
  bool p1_win = (gd->p2_board & gd->p1_hit_board) == gd->p2_board;
  // or the opposite
  bool p2_win = (gd->p1_board & gd->p2_hit_board) == gd->p1_board;

  // both should never be true at the same time
  assert(!(p2_win && p1_win));

  if (p1_win) {
    gd->winner = P1;
    return TRUE;
  }

  if (p2_win) {
    gd->winner = P2;
    return TRUE;
  }

  return FALSE;
}

game_data *start_new_game(int idx, client *player1, client *player2)
{
  game_data *game = malloc(sizeof(game_data));
  bzero(game, sizeof(game_data));
  game->players[0] = player1;
  game->players[1] = player2;

  player1->game_id = idx;
  player1->player_idx = 0;

  player2->game_id = idx;
  player2->player_idx = 0;

  // send start game messages to clients
  return game;
}

int send_game_msg(game_msg_op opcode, char *msg, int len, client *conn,
                  game_data *gd, int broadcast)
{
  ws_frame *frame = malloc(sizeof(ws_frame));
  bzero(frame, sizeof(ws_frame));
  frame->opcode = 2;
  frame->msg_len = len + 1; // +1 for game msg opcode
  frame->msg[0] = (0x0F & opcode) << 4;
  memcpy(frame->msg + 1, msg, len);

  int len_bytes = 0;
  unsigned char *frame_bytes = ws_frame_to_bytes(frame, &len_bytes);

  if (broadcast) {
    printf("Broadcasting msg: ");
    for (int i = 0; i < len_bytes; i++) {
      printf("%c", frame_bytes[i]);
    }
    printf("\n");

    for (int i = 0; i < 2; i++) {
      int n = write(gd->players[i]->fd, frame_bytes, len_bytes);
      if (n <= 0) {
        perror("Error sending msg to player");
      }
    }
  }
  else {
    printf("Sending msg to player %d: ", conn->player_idx);
    for (int i = 0; i < len_bytes; i++) {
      printf("%c", frame_bytes[i]);
    }

    printf("\n");
    int n = write(conn->fd, frame_bytes, len_bytes);
    if (n <= 0) {
      perror("Error sending msg to player");
    }
  }

  free(frame);
  free(frame_bytes);
  return 0;
}

int handle_game_msg(server_ctx *ctx, unsigned char ws_data[MAX_WS_MSG_SIZE],
                    client *conn)
{
  int opcode = (ws_data[0] & 0xF0) >> 4;

  // Find user's game if exists
  game_data *gd = NULL;
  if (conn->game_id > MAX_GAMES) {
    fprintf(stderr, "Error: invalid game_id %d for connection %d\n",
            conn->game_id, conn->fd);
    return 1;
  }
  else if (conn->game_id >= 0) {
    gd = ctx->games[conn->game_id];
    if (gd == NULL) {
      fprintf(stderr, "Error: game_id %d does not exist for connection %d\n",
              conn->game_id, conn->fd);
      send_game_msg(GAME_MSG_ERROR, "Error: game invalid",
                    strlen("Error: game invalid"), conn, gd, 0);
      return 2;
    }
  }

  if (gd == NULL && opcode != GAME_MSG_READY) {
    send_game_msg(GAME_MSG_ERROR, "Invalid Msg: player not in game",
                  strlen("Invalid Msg: player not in game"), conn, gd, 0);
    return 3;
  }

  switch (opcode) {
  case GAME_MSG_BOARD_SETUP:
    printf("Board setup game message received\n");

    unsigned char *board_str = ws_data + 1;
    int err = parse_validate_board(board_str, gd->p1_ships, &gd->p1_board);
    if (err != 0) {
      send_game_msg(GAME_MSG_ERROR, "Invalid Board", strlen("Invalid Board"),
                    conn, gd, 0);
      return 0;
    }

    send_game_msg(GAME_MSG_BOARD_SETUP, "Success", strlen("Success"), conn, gd,
                  0);
    break;
  case GAME_MSG_READY:
    if (ctx->num_games < MAX_GAMES) {
      ctx->games[ctx->num_games] = start_new_game(ctx->num_games, conn, conn);
      ctx->num_games++;
    }
    send_game_msg(GAME_MSG_READY, "Success", strlen("Success"), conn, gd, 0);
    break;
    // add more here for other msg types (shot, resign, etc)
  default:
    send_game_msg(GAME_MSG_ERROR, "Invalid Msg", strlen("Invalid Msg"), conn,
                  gd, 0);
    printf("Unrecognized game msg\n");
    break;
  }
  return 0;
}

#include "defs.h"

int get_open_game_slot() {
  for (int i = 0; i < MAX_GAMES; ++i) {
    if (server.games[i] == NULL) return i;
  }
  return -1;
}