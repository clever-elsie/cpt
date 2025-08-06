#pragma once
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <string>
#include <variant>
namespace mp=boost::multiprecision;
using bint=mp::cpp_int;
using bfloat=mp::cpp_dec_float_50;
using expr_t = std::variant<bint,bfloat,bool>;

expr_t operator+=(expr_t&lhs,expr_t&rhs);
expr_t operator+=(expr_t&lhs,expr_t&&rhs);
expr_t operator-=(expr_t&lhs,expr_t&rhs);
expr_t operator-=(expr_t&lhs,expr_t&&rhs);
expr_t operator*=(expr_t&lhs,expr_t&rhs);
expr_t operator*=(expr_t&lhs,expr_t&&rhs);
expr_t operator/=(expr_t&lhs,expr_t&rhs);
expr_t operator/=(expr_t&lhs,expr_t&&rhs);
expr_t operator%=(expr_t&lhs,expr_t&rhs);
expr_t operator%=(expr_t&lhs,expr_t&&rhs);
bool operator==(const expr_t&lhs,const expr_t&rhs);
bool operator<(const expr_t&lhs,const expr_t&rhs);
bool operator&&(const expr_t&lhs,const expr_t&rhs);
bool operator||(const expr_t&lhs,const expr_t&rhs);
expr_t pow(expr_t&lhs, expr_t&rhs);
bool to_bool(const expr_t&value);