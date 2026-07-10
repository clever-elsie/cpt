#include "ast/ast.hpp"

namespace AST{

Nitem::Nitem(size_t row,size_t col):row(row),col(col){}

void Nitem::error_exit(std::string_view msg){
  throw std::runtime_error(std::to_string(row)+":"+std::to_string(col)+": error: "+std::string(msg));
}

std::pair<size_t,size_t> Nitem::get_pos()const{
  return {row,col};
}

} // namespace AST