#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <fmt/core.h>

using Block = std::vector<std::vector<bool>>;
using CharBlock = std::vector<std::vector<char>>;

Block read_board(std::fstream &towers) {
  Block board;
  std::string line;
  while (std::getline(towers, line) && !line.empty()) {
    std::vector<bool> row(line.size());
    for (std::size_t i = 0; i < line.size(); ++i) {
      row[i] = (line[i] == 'X');
    }
    board.emplace_back(std::move(row));
  }
  return board;
}

std::vector<Block> create_boards(Block const &board) {
  std::size_t const num_rows = board.size();
  std::size_t const num_cols = board[0].size();

  Block empty_board(num_rows);
  for (auto &row : empty_board) {
    row.resize(num_cols);
  }

  std::vector<Block> boards;
  for (std::size_t row_idx = 0; row_idx < num_rows; ++row_idx) {
    for (std::size_t col_idx = 0; col_idx < num_cols; ++col_idx) {
      if (board[row_idx][col_idx]) {
        Block empty_board_cpy = empty_board;
        empty_board_cpy[row_idx][col_idx] = true;
        boards.emplace_back(std::move(empty_board_cpy));
      }
    }
  }

  return boards;
}

std::vector<Block> read_pieces(std::fstream &towers) {
  std::vector<Block> pieces;
  Block matrix;
  std::string line;
  while (std::getline(towers, line)) {
    if (line.empty()) {
      pieces.emplace_back(std::move(matrix));
      continue;
    }
    std::vector<bool> row(line.size());
    for (std::size_t i = 0; i < line.size(); ++i) {
      row[i] = (line[i] == 'X');
    }
    matrix.emplace_back(std::move(row));
  }
  pieces.emplace_back(std::move(matrix));
  return pieces;
}

Block rotate(Block const &piece) {
  std::size_t const num_rows = piece.size();
  std::size_t const num_cols = piece[0].size();

  std::vector<std::vector<bool>> rot_piece(num_cols);
  for (auto &row : rot_piece) {
    row.resize(num_rows);
  }

  for (std::size_t row = 0; row < num_cols; ++row) {
    for (std::size_t col = 0; col < num_rows; ++col) {
      rot_piece[row][col] = piece[num_rows - col - 1][row];
    }
  }

  return rot_piece;
}

void print(Block const &piece) {
  for (auto const &row : piece) {
    for (auto const &col : row) {
      std::cout << (col ? 'X' : '-');
    }
    std::cout << '\n';
  }
  std::cout << '\n';
}

void print(CharBlock const &piece) {
  for (auto const &row : piece) {
    for (auto const &col : row) {
      std::cout << col;
    }
    std::cout << '\n';
  }
  std::cout << '\n';
}

bool can_mask(
    std::size_t row,
    std::size_t col,
    Block const &board,
    Block const &piece) {
  std::size_t const num_rows = piece.size();
  std::size_t const num_cols = piece[0].size();

  if (row + num_rows > board.size() || col + num_cols > board[0].size()) {
    return false;
  }

  for (std::size_t i = 0; i < num_rows; ++i) {
    for (std::size_t j = 0; j < num_cols; ++j) {
      if (piece[i][j] && board[row + i][col + j]) {
        return false;
      }
    }
  }

  return true;
}

void mask(
    std::size_t row,
    std::size_t col,
    Block &board,
    Block const &piece,
    bool value) {
  std::size_t const num_rows = piece.size();
  std::size_t const num_cols = piece[0].size();

  for (std::size_t i = 0; i < num_rows; ++i) {
    for (std::size_t j = 0; j < num_cols; ++j) {
      if (piece[i][j]) {
        board[row + i][col + j] = value;
      }
    }
  }
}

void mask(
    std::size_t row,
    std::size_t col,
    CharBlock &board,
    Block const &piece,
    char ch) {
  std::size_t const num_rows = piece.size();
  std::size_t const num_cols = piece[0].size();

  for (std::size_t i = 0; i < num_rows; ++i) {
    for (std::size_t j = 0; j < num_cols; ++j) {
      if (piece[i][j]) {
        board[row + i][col + j] = ch;
      }
    }
  }
}

bool place(
    Block &board,
    CharBlock &char_board,
    std::vector<Block>::iterator begin,
    std::vector<Block>::iterator end,
    std::size_t depth = 1) {
  if (begin == end) {
    return true;
  }

  Block piece = *begin;

  std::size_t const board_rows = board.size();
  std::size_t const board_cols = board[0].size();

  for (int rot = 0; rot < 4; ++rot) {
    for (std::size_t row = 0; row < board_rows; ++row) {
      for (std::size_t col = 0; col < board_cols; ++col) {
        if (can_mask(row, col, board, piece)) {
          mask(row, col, board, piece, true);
          bool finished = place(board, char_board, std::next(begin), end, depth + 1);
          if (finished) {
            mask(row, col, char_board, piece, 'X');
            print(char_board);
            mask(row, col, char_board, piece, '-');
            return true;
          }
          mask(row, col, board, piece, false);
        }
      }
    }
    piece = rotate(piece);
  }
  return false;
}

std::size_t block_size(Block const &block) {
  std::size_t count{};
  for (auto const &row : block) {
    count += static_cast<std::size_t>(std::count(row.begin(), row.end(), true));
  }
  return count;
}

void inc_place(Block board, std::vector<Block> pieces) {
  std::sort(
      pieces.begin(),
      pieces.end(),
      [](Block const &piece_l, Block const &piece_r) {
        return block_size(piece_l) < block_size(piece_r);
      });

  auto const num_rows = board.size();
  auto const num_cols = board[0].size();
  CharBlock char_board(num_rows);
  for (auto &char_row : char_board) {
    char_row.resize(num_cols, '-');
  }

  print(board);
  place(board, char_board, pieces.begin(), pieces.end());
}

void dec_place(Block board, std::vector<Block> pieces) {
  std::sort(
      pieces.begin(),
      pieces.end(),
      [](Block const &piece_l, Block const &piece_r) {
        return block_size(piece_l) > block_size(piece_r);
      });

  auto const num_rows = board.size();
  auto const num_cols = board[0].size();
  CharBlock char_board(num_rows);
  for (auto &char_row : char_board) {
    char_row.resize(num_cols, '-');
  }

  print(board);
  place(board, char_board, pieces.begin(), pieces.end());
  fmt::println("==========\n");
}

int main() {
  std::fstream towers("towers.txt");
  if (!towers.is_open()) {
    std::cerr << "Couldn't open towers.txt\n";
    return 1;
  }

  Block main_board = read_board(towers);
  std::vector<Block> boards = create_boards(main_board);
  std::vector<Block> pieces = read_pieces(towers);

  for (Block const &board : boards) {
    dec_place(board, pieces);
  }

  return 0;
}
