#pragma once
#include "top.hpp"

namespace EXPR{
AST::Nitem* expr(tokenize&tok);
AST::Nitem* expr_3(tokenize&tok);
AST::Nitem* or_expr(tokenize&tok);
AST::Nitem* and_expr(tokenize&tok); 
AST::Nitem* compare(tokenize&tok);
AST::Nitem* algebra(tokenize&tok);
AST::Nitem* term(tokenize&tok);
AST::Nitem* factor(tokenize&tok);
AST::Nitem* atom(tokenize&tok);
} // namespace EXPR