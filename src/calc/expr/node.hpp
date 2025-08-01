#pragma once
#include "expr.hpp"

namespace CALC{
bool top(std::string_view istr);
expr_t second(tokenize&tok,bool is_fn);
namespace EXPR{
expr_t expr_3(tokenize&tok);
expr_t or_expr(tokenize&tok);
expr_t and_expr(tokenize&tok); 
expr_t compare(tokenize&tok);
expr_t algebra(tokenize&tok);
expr_t term(tokenize&tok);
expr_t factor(tokenize&tok);
expr_t atom(tokenize&tok);
expr_t reserved_functions(tokenize&tok);
expr_t parse_DECIMAL(tokenize&tok);
expr_t parse_BINARY(tokenize&tok);
expr_t parse_HEX(tokenize&tok);
expr_t parse_FLOAT(tokenize&tok);
}}//namespace EXPR}CALC}