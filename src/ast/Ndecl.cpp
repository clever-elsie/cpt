#include "ast.hpp"

namespace AST{

Ndecl::Ndecl(std::string_view name,Nitem*init,Nstat*belong_to)
:name(name),init(init),belong_to(belong_to){
  is_local_first=belong_to->args_set.contains(name)||belong_to->var_names_set.contains(name);
}

Ndecl::~Ndecl(){
  delete init;
  init=nullptr;
}

std::string_view Ndecl::get_name()const{
  return name;
}

expr_t Ndecl::get_value(){
  // 初期値を評価．変数テーブルに登録
  expr_t value=init->get_value();
  if(is_local_first){
    belong_to->var_names.push_back(name);
    auto itr=var_map.find(name);
    if(itr==var_map.end())
      itr=var_map.emplace(name,std::vector<expr_t>()).first;
    itr->second.emplace_back(std::move(value));
  }else{
    auto itr=var_map.find(name);
    if(itr==var_map.end())
      throw std::runtime_error("変数が見つかりません");
    itr->second.back()=std::move(value);
  }
  return value;
}

void Ndecl::move_to(Nstat*belong_to){
  this->belong_to=belong_to;
  if(is_local_first=belong_to->var_names_set.emplace(name).second)
    belong_to->var_names.push_back(name);
}

} // namespace AST