#include "ast.hpp"
#include <ranges>
#include <algorithm>

namespace AST{

expr_t Nstat::evaluate(std::vector<expr_t>&&args){
  if(args.size()!=this->args.size())
    throw std::runtime_error("引数の数が一致しません");
  for(auto&&[aname,aexpr]:std::views::zip(this->args,args))
    var_map[aname].emplace_back(std::move(aexpr));
  expr_t result;
  for(auto&&item:items)
    result=item->get_value();
  for(auto&&arg:this->args){
    auto&v=var_map[arg];
    if(v.empty()) throw std::runtime_error("引数の数が一致しません");
    v.pop_back();
  }
  for(auto&&var:var_names){
    auto&v=var_map[var];
    if(v.empty()) throw std::runtime_error("引数の数が一致しません");
    v.pop_back();
  }
  return result;
}

Nstat::~Nstat(){
  for(auto&&item:items) delete item;
}

} // namespace AST