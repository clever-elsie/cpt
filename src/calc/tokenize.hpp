#pragma once
#include "type.hpp"

namespace CALC{
class tokenize{
  pToken first;
  std::string_view istr;
  public:
  tokenize()=delete;
  tokenize(const tokenize&)=default;
  tokenize(tokenize&&)=default;
  tokenize(std::string_view istr)noexcept(false);
  tokenize&operator=(const tokenize&)=default;
  tokenize&operator=(tokenize&&)=default;
  ~tokenize()=default;
  pToken next_token()noexcept(false);
  pToken top()noexcept(false);
  private:
  pToken get_token()noexcept(false);
  pToken get_number()noexcept(false);
  pToken get_ident()noexcept(true);
};
}// namespace