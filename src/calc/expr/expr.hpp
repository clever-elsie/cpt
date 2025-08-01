#pragma once
#include "../tokenize.hpp"
#include "../type.hpp"

namespace CALC{ namespace EXPR{
inline VAR_MAP<expr_t> var_map;
inline FN_MAP fn_map;
expr_t expr(tokenize&tok);
}} // namespace EXPR}CALC}