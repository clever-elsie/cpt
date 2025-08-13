#pragma once
#include <cstdlib>
#include <string>
#include <string_view>
#include <cctype>
#include <iostream>

enum class except{
  EMPTY,
  INVALID_TOKEN,
};

enum class token_t{
  DECIMAL, HEX, BINARY, FLOAT,
  RESERVED, IDENT, SYMBOL, EMPTY
};

enum class symbol_t{
  NAS, // not a symbol
  PLUS, MINUS, MUL, POW, // + - * **
  FDIV, IDIV, MOD, // / // %
  LAND, LOR, // && &, || |
  EXCL, QUEST, // ! ?
  ASSIGN, EQ, EEQ, // := = ==
  NE, LT, LE, GT, GE, // != < <= > >=
  LPAREN, RPAREN, // ()
  LSQUARE, RSQUARE, // []
  LCURLY, RCURLY, // {}
  COMMA, DOT, SEMICOLON, COLON, // , . ; :
  UNDERSCORE, // _
  CARET, // ^
};

struct pToken{
  token_t type;
  symbol_t symbol;
  size_t len;
  std::string_view token;
};

class tokenize{
  std::string_view::const_iterator prev_linehead;
  pToken first;
  std::string_view istr;
  size_t row,col;
  public:
  tokenize()=delete;
  tokenize(const tokenize&)=default;
  tokenize(tokenize&&)=default;
  tokenize(std::string_view istr)noexcept;
  tokenize&operator=(const tokenize&)=default;
  tokenize&operator=(tokenize&&)=default;
  ~tokenize()=default;
  pToken next_token()noexcept;
  pToken top()noexcept;
  void error_exit(const std::string_view&msg)noexcept;
  void error_throw(const std::string&msg)noexcept(false);
  std::pair<size_t,size_t> get_pos()const noexcept;
  private:
  std::string gen_error_msg(const std::string_view&msg)const;
  pToken get_token()noexcept;
  pToken get_number()noexcept;
  pToken get_ident()noexcept;
  bool skip_whitespace_and_comment()noexcept;
};