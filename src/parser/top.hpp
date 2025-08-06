#pragma once
#include <string_view>
#include "../ast/ast.hpp"
#include "../tokenizer/tokenize.hpp"

namespace PARSER{
AST::Nstat* top(std::string_view istr);
AST::Ndecl* define_var(tokenize&tok,AST::Nstat*parent);
}