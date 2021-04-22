#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include "defs.h"

using namespace std;

struct StackEntry { /// info to keep in the stack
  uint16_t move, piece;
  int eval;
};

inline void join(vector <uint16_t> &a, vector <uint16_t> b) {
  a.insert(a.end(), b.begin(), b.end());
}

class Undo {
public:
  int castleRights;
  int halfMoves, moveIndex;
  int enPas;
  int captured;
  //uint64_t checkers;
  uint64_t key;
};

class Board {
public:
  uint8_t board[64];
  uint64_t bb[13];
  uint64_t pieces[2];
  //uint64_t checkers;
  int castleRights; /// 1 - bq, 2 - bk, 4 - wq, 8 - wk
  Undo history[400];

  bool turn;
  int captured; /// keeping track of last captured piece so i reduce the size of move
  int ply, gamePly;
  int enPas;
  int halfMoves, moveIndex;
  uint64_t key;

  Board() {
    halfMoves = moveIndex = turn = key = 0;
    ply = gamePly = 0;
    //checkers = 0;
    for(int i = 0; i <= 12; i++)
      bb[i] = 0;
    for(int i = 0; i < 64; i++)
      board[i] = 0;
    castleRights = 0;
    captured = 0;
  }

  Board(const Board &other) {
    halfMoves = other.halfMoves;
    moveIndex = other.moveIndex;
    turn = other.turn;
    key = other.key;
    gamePly = other.gamePly;
    ply = other.ply;
    //checkers = other.checkers;
    for(int i = 0; i <= 12; i++)
      bb[i] = other.bb[i];
    for(int i = BLACK; i <= WHITE; i++)
      pieces[i] = other.pieces[i];
    for(int i = 0; i < 64; i++)
      board[i] = other.board[i];
    castleRights = other.castleRights;
    captured = other.captured;
  }

  uint64_t diagSliders(int color) {
    return bb[getType(BISHOP, color)] | bb[getType(QUEEN, color)];
  }

  uint64_t orthSliders(int color) {
    return bb[getType(ROOK, color)] | bb[getType(QUEEN, color)];
  }

  int piece_type_at(int sq) {
    return piece_type(board[sq]);
  }

  int piece_at(int sq) {
    return board[sq];
  }

  /*int color_at(int sq) {
    if(board[sq] == 0) /// empty square
      return -1;
    return board[sq] / 7;
  }*/

  int king(int color) {
    return 63 - __builtin_clzll(bb[BK + color * 6]);
  }

  bool isCapture(int move) {
    return (board[sqTo(move)] > 0);
  }

  void clear() {
    ply = 0;
  }

  void print() {
    for(int i = 7; i >= 0; i--) {
      for(int j = 0; j <= 7; j++)
        cout << piece[board[8 * i + j]] << " ";
      cout << "\n";
    }
  }

  void setFen(const string fen) {
    int ind = 0;
    key = 0;

    ply = gamePly = 0;
    captured = 0;

    //checkers = 0;

    //cout << "XD?\n";

    for(int i = 0; i <= 12; i++)
      bb[i] = 0;

    pieces[BLACK] = pieces[WHITE] = 0;

    for(int i = 7; i >= 0; i--) {
      int j = 0;
      while(fen[ind] != '/' && fen[ind] != ' ') {
        int sq = getSq(i, j);
        if(fen[ind] < '0' || '9' < fen[ind]) {
          board[sq] = cod[fen[ind]];
          key ^= hashKey[board[sq]][sq];
          //cout << "INIT " << fen[ind] << " " << sq << " " << cod[fen[ind]] << " " << hashKey[cod[fen[ind]]][sq] << "\n";
          pieces[(board[sq] > 6)] |= (1ULL << sq);
          bb[board[sq]] |= (1ULL << sq);
          j++;
        } else {
          int nr = fen[ind] - '0';
          while(nr)
            board[sq] = 0, j++, sq++, nr--;
        }
        ind++;
      }
      ind++;
    }

    if(fen[ind] == 'w')
      turn = WHITE;
    else
      turn = BLACK;

    key ^= turn;

    //cout << "key " << key << "\n";

    castleRights = 0;
    ind += 2;
    if(fen[ind] == 'K')
      castleRights |= (1 << 3), ind++, key ^= castleKey[1][1];
    if(fen[ind] == 'Q')
      castleRights |= (1 << 2), ind++, key ^= castleKey[1][0];
    if(fen[ind] == 'k')
      castleRights |= (1 << 1), ind++, key ^= castleKey[0][1];
    if(fen[ind] == 'q')
      castleRights |= (1 << 0), ind++, key ^= castleKey[0][0];
    if(fen[ind] == '-')
      ind++;


    //cout << "key " << key << "\n";
    ind++;
    if(fen[ind] != '-') {
      int file = fen[ind] - 'a';
      ind++;
      int rank = fen[ind] - '1';
      ind += 2;
      enPas = getSq(rank, file);

      key ^= enPasKey[enPas];
    } else {
      enPas = -1;
      ind += 2;
    }
    //cout << "key " << key << "\n";

    int nr = 0;
    while('0' <= fen[ind] && fen[ind] <= '9')
      nr = nr * 10 + fen[ind] - '0', ind++;
    halfMoves = nr;

    ind++;
    nr = 0;
    while('0' <= fen[ind] && fen[ind] <= '9')
      nr = nr * 10 + fen[ind] - '0', ind++;
    moveIndex = nr;
  }

  string fen() {
    string fen = "";
    for(int i = 7; i >= 0; i--) {
      int cnt = 0;
      for(int j = 0, sq = i * 8; j < 8; j++, sq++) {
        if(board[sq] == 0)
          cnt++;
        else {
          if(cnt)
            fen += char(cnt + '0');
          cnt = 0;
          fen += piece[board[sq]];
        }
      }
      if(cnt)
        fen += char(cnt + '0');
      if(i)
        fen += "/";
    }
    fen += " ";
    fen += (turn == WHITE ? "w" : "b");
    fen += " ";
    if(castleRights & 8)
      fen += "K";
    if(castleRights & 4)
      fen += "Q";
    if(castleRights & 2)
      fen += "k";
    if(castleRights & 1)
      fen += "q";
    if(!castleRights)
      fen += "-";
    fen += " ";
    if(enPas >= 0) {
      fen += char('a' + enPas % 8);
      fen += char('1' + enPas / 8);
    } else
      fen += "-";
    fen += " ";
    string s = "";
    int nr = halfMoves;
    while(nr)
      s += char('0' + nr % 10), nr /= 10;
    reverse(s.begin(), s.end());
    if(halfMoves)
      fen += s;
    else
      fen += "0";
    fen += " ";
    s = "", nr = moveIndex;
    while(nr)
      s += char('0' + nr % 10), nr /= 10;
    reverse(s.begin(), s.end());
    fen += s;
    return fen;
  }

  uint64_t hash() {
    uint64_t h = 0;
    h ^= turn;
    for(int i = 0; i < 64; i++) {
      if(board[i])
        h ^= hashKey[board[i]][i];
    }
    //cout << h << "\n";
    for(int i = 0; i < 4; i++)
      h ^= castleKey[i / 2][i % 2] * ((castleRights >> i) & 1);
    //cout << h << "\n";

    h ^= (enPas >= 0 ? enPasKey[enPas] : 0);
    //cout << h << "\n";
    return h;
  }

  bool isMaterialDraw() {
    if(count(pieces[WHITE]) == 1 && count(pieces[BLACK]) == 1)
      return 1;
    if(count(pieces[WHITE]) == 2 && count(pieces[BLACK]) == 1) {
      if(bb[WN] || bb[WB])
        return 1;
    }
    if(count(pieces[BLACK]) == 2 && count(pieces[WHITE]) == 1) {
      if(bb[BN] || bb[BB])
        return 1;
    }
    return 0;
  }
};

class Info {
public:
  long double startTime, stopTime;
  int depth, sel_depth, multipv;
  int timeset;
  int movestogo;
  long long nodes;

  bool quit, stopped;
};