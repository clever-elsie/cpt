#include "ast/ast.hpp"
#include <ranges>
#include <algorithm>

namespace AST{

Nstat::Nstat() : Nitem(0, 0) {}

expr_t Nstat::get_value() {
  return evaluate({});
}

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

json::value Nstat::to_json() const {
  json::value v;
  v["type"] = "stat";
  json::value::array_t arr;
  for (auto child : items) {
    arr.push_back(child ? child->to_json() : json::value());
  }
  v["items"] = arr;
  json::value::array_t vars;
  for (const auto& var : var_names) {
    vars.push_back(json::value(var));
  }
  v["var_names"] = vars;
  v["row"] = (int64_t)row;
  v["col"] = (int64_t)col;
  return v;
}

} // namespace AST