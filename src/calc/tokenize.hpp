#pragma once
#include "type.hpp"
#include <cstdlib>

namespace CALC{
class tokenize{
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
  private:
  pToken get_token()noexcept;
  pToken get_number()noexcept;
  pToken get_ident()noexcept;
};
}// namespace