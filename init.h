#pragma once
#include <string>
#include "board.h"
#include "defs.h"
#include "move.h"
#include "attacks.h"

const char initBoard[] = {
  'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R',
  'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P',
  '.', '.', '.', '.', '.', '.', '.', '.',
  '.', '.', '.', '.', '.', '.', '.', '.',
  '.', '.', '.', '.', '.', '.', '.', '.',
  '.', '.', '.', '.', '.', '.', '.', '.',
  'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p',
  'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r',
};

void init(Info *info) {
  info->quit = info->stopped = 0;
  info->depth = DEPTH;
  info->multipv = 1;
}

void init(Board &board, std::string fen = START_POS_FEN) {

  //init_defs();

  //initAttacks();

  board.setFen(fen);
  board.ply = board.gamePly = 0;

  for(int i = 0; i <= 12; i++)
    board.bb[i] = 0;

  /*for(int i = 0; i <= 12; i++) {
    for(int j = 0; j < 64; j++)
      board.hist[0][i][j] = board.hist[1][i][j] = NULLMOVE;
  }

  for(int i = 0; i < 100; i++)
    board.killers[i][0] = board.killers[i][1] = NULLMOVE;*/

  board.pieces[WHITE] = board.pieces[BLACK] = 0;

  for(int i = 0; i < 64; i++) {
    int piece = board.piece_at(i);
    if(piece)
      board.bb[piece] |= (1ULL << i), board.pieces[board.board[i] > 6] |= (1ULL << i);
  }
}
