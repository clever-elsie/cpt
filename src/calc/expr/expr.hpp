#pragma once
#include "../tokenize.hpp"
#include "../type.hpp"

namespace CALC{ namespace EXPR{
inline VAR_MAP<expr_t> var_map;
expr_t expr(tokenize&tok) noexcept(false);
}} // namespace EXPR}CALC}