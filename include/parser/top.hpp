#pragma once
#include <string_view>
#include "ast/ast.hpp"
#include "tokenizer/tokenize.hpp"

namespace PARSER{
AST::Nstat* top(std::string_view istr, std::string_view file_name = "<stdin>");
void second(tokenize&tok,bool is_fn,AST::Nstat*parent,std::string_view ns_prefix = "");
AST::Ndecl* define_var(tokenize&tok,AST::Nstat*parent,std::string_view ns_prefix = "");
void define_fn(tokenize&tok,std::string_view ns_prefix = "");
}