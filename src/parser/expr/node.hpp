#pragma once
#include "expr.hpp"

AST::Nstat* top(std::string_view istr);
void second(tokenize&tok,bool is_fn,AST::Nstat*parent);

namespace EXPR{
AST::Nitem* expr_3(tokenize&tok);
AST::Nitem* or_expr(tokenize&tok);
AST::Nitem* and_expr(tokenize&tok); 
AST::Nitem* compare(tokenize&tok);
AST::Nitem* algebra(tokenize&tok);
AST::Nitem* term(tokenize&tok);
AST::Nitem* factor(tokenize&tok);
AST::Nitem* atom(tokenize&tok);
AST::Nitem* parse_DECIMAL(tokenize&tok);
AST::Nitem* parse_BINARY(tokenize&tok);
AST::Nitem* parse_HEX(tokenize&tok);
AST::Nitem* parse_FLOAT(tokenize&tok);
}//namespace EXPR