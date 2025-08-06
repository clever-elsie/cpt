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
  for(auto&&arg:this->args) var_map[arg].pop_back();
  for(auto&&var:var_names) var_map[var].pop_back();
  return result;
}

Nstat::~Nstat(){
  for(auto&&item:items) delete item;
}

} // namespace AST