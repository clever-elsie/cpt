#pragma once
#include "../../node.hpp"

namespace CALC{namespace EXPR{
expr_t sum(tokenize&tok);
expr_t prod(tokenize&tok);
expr_t log(tokenize&tok);
expr_t log10(tokenize&tok);
expr_t abs(tokenize&tok);
expr_t cos(tokenize&tok);
expr_t sin(tokenize&tok);
expr_t tan(tokenize&tok);
expr_t acos(tokenize&tok);
expr_t asin(tokenize&tok);
expr_t atan(tokenize&tok);
expr_t cosh(tokenize&tok);
expr_t sinh(tokenize&tok);
expr_t tanh(tokenize&tok);
expr_t print(tokenize&tok);
}}// namespace EXPR}CALC}