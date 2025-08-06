#include "ast.hpp"

namespace AST{

Nvar::Nvar(size_t row,size_t col,std::string_view name):Nitem(row,col),name(name){}

expr_t Nvar::get_value(){
  auto it=var_map.find(name);
  if(it==var_map.end()||it->second.empty())
    throw std::runtime_error("未定義の変数"+std::string(name)+"を参照しています");
  return it->second.back();
}

std::string_view Nvar::get_name()const{
  return name;
}

} // namespace AST