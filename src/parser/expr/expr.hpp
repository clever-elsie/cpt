#pragma once
#include "../../tokenizer/tokenize.hpp"
#include "../../type/expr_t.hpp"
#include "../../ast/ast.hpp"

namespace EXPR{
AST::Nitem* expr(tokenize&tok);
} // namespace EXPR