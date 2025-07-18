#pragma once
#include "../../node.hpp"

namespace CALC{namespace EXPR{
expr_t sum(tokenize&tok)noexcept(false);
expr_t prod(tokenize&tok)noexcept(false);
expr_t log(tokenize&tok)noexcept(false);
expr_t log10(tokenize&tok)noexcept(false);
expr_t abs(tokenize&tok)noexcept(false);
expr_t cos(tokenize&tok)noexcept(false);
expr_t sin(tokenize&tok)noexcept(false);
expr_t tan(tokenize&tok)noexcept(false);
expr_t acos(tokenize&tok)noexcept(false);
expr_t asin(tokenize&tok)noexcept(false);
expr_t atan(tokenize&tok)noexcept(false);
expr_t cosh(tokenize&tok)noexcept(false);
expr_t sinh(tokenize&tok)noexcept(false);
expr_t tanh(tokenize&tok)noexcept(false);
expr_t print(tokenize&tok)noexcept(false);
}}// namespace EXPR}CALC}