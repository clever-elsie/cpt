#pragma once
#include "expr.hpp"

namespace CALC{namespace EXPR{
expr_t expr_3(tokenize&tok) noexcept(false);
expr_t or_expr(tokenize&tok) noexcept(false);
expr_t and_expr(tokenize&tok) noexcept(false); 
expr_t compare(tokenize&tok) noexcept(false);
expr_t algebra(tokenize&tok) noexcept(false);
expr_t term(tokenize&tok) noexcept(false);
expr_t factor(tokenize&tok) noexcept(false);
expr_t atom(tokenize&tok) noexcept(false);
expr_t reserved_functions(tokenize&tok)noexcept(false);
expr_t parse_DECIMAL(tokenize&tok)noexcept(true);
expr_t parse_BINARY(tokenize&tok)noexcept(true);
expr_t parse_HEX(tokenize&tok)noexcept(true);
expr_t parse_FLOAT(tokenize&tok)noexcept(true);
}}//namespace EXPR}CALC}