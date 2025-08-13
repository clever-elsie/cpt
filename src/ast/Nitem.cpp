#include "ast/ast.hpp"

namespace AST{

Nitem::Nitem(size_t row,size_t col):row(row),col(col){}

void Nitem::error_exit(std::string_view msg){
  std::cerr<<row<<":"<<col<<":"<<std::string(msg)<<std::endl;
  std::exit(EXIT_FAILURE);
}

std::pair<size_t,size_t> Nitem::get_pos()const{
  return {row,col};
}

} // namespace AST