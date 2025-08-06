#include "ast.hpp"

namespace AST{

Nliteral::Nliteral(size_t row,size_t col,expr_t value)
:Nitem(row,col),value(value){}

expr_t Nliteral::get_value(){
  return value;
}

} // namespace AST