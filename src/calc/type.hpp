#pragma once
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <string>
#include <variant>
#include <string_view>
#include "var_map.hpp"
namespace CALC{
enum class except{
  EMPTY,
  INVALID_TOKEN,
};
enum class token_t{
  DECIMAL, HEX, BINARY, FLOAT,
  RESERVED, IDENT, SYMBOL, EMPTY
};
struct pToken{
  token_t type;
  std::string_view token;
};

namespace mp=boost::multiprecision;
using bint=mp::cpp_int;
using bfloat=mp::cpp_dec_float_50;
using expr_t = std::variant<bint,bfloat,bool>;

expr_t operator+=(expr_t&lhs,expr_t&&rhs);
expr_t operator-=(expr_t&lhs,expr_t&&rhs);
expr_t operator*=(expr_t&lhs,expr_t&&rhs);
expr_t operator/=(expr_t&lhs,expr_t&&rhs);
bool operator==(expr_t&lhs,expr_t&rhs);
bool operator<(expr_t&lhs,expr_t rhs);
expr_t pow(const expr_t&lhs, const expr_t& rhs);

}// namespace CALC