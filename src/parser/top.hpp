#pragma once
#include <string_view>
#include "../ast/ast.hpp"
#include "../tokenizer/tokenize.hpp"

namespace PARSER{
AST::Nstat* top(std::string_view istr);
void second(tokenize&tok,bool is_fn,AST::Nstat*parent);
AST::Ndecl* define_var(tokenize&tok,AST::Nstat*parent);
void define_fn(tokenize&tok);
}