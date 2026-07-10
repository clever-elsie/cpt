#include "ast/ast.hpp"
#include "ast/reserved.hpp"

namespace AST{

Nfn::Nfn(size_t row,size_t col,std::string_view name,std::vector<Nitem*>&&args)
:Nitem(row,col),name(name),args(std::move(args)){}

Nfn::~Nfn(){
  for(auto&&arg:args) delete arg;
  args.clear();
}

expr_t Nfn::get_value(){
  if(name.starts_with("\\")) return eval_reserved_fn();
  
  Nstat* fn_stat = nullptr;
  std::unordered_map<std::string, expr_t> closure;
  bool is_lambda = false;

  auto fn=fn_map.find(name);
  if(fn != fn_map.end()){
    fn_stat = fn->second;
  } else {
    auto var = var_map.find(name);
    if(var != var_map.end() && !var->second.empty()){
      expr_t val = var->second.back();
      if(val.is<expr_t::types::FUNCTION>()){
        auto l = val.get<std::shared_ptr<LambdaFunc>>();
        fn_stat = l->body;
        closure = l->closure_env;
        is_lambda = true;
      }
    }
  }

  if(!fn_stat) throw std::runtime_error("未定義の関数: " + name);

  std::vector<expr_t> args_eval;
  for(auto&&arg:this->args)
    args_eval.push_back(arg->get_value());
  
  if(is_lambda){
    for(const auto& [vname, vval] : closure){
      var_map[vname].push_back(vval);
    }
  }

  expr_t ret;
  try {
    ret = fn_stat->evaluate(std::move(args_eval));
  } catch (...) {
    if(is_lambda){
      for(const auto& [vname, vval] : closure){
        var_map[vname].pop_back();
      }
    }
    throw;
  }

  if(is_lambda){
    for(const auto& [vname, vval] : closure){
      var_map[vname].pop_back();
    }
  }
  return ret;
}

constexpr std::array<std::pair<std::string_view,expr_t(*)(std::vector<Nitem*>&)>,46>
construct_reserved_fnlist(){
  using fn_t=expr_t(*)(std::vector<Nitem*>&);
  using fn_p_t=std::pair<std::string_view,fn_t>;
  #define def(name) fn_p_t{#name, name}
  std::array<fn_p_t,46> list{
    def(log),def(sum),def(prod),def(abs),def(log10),
    def(cos),def(sin),def(tan),def(acos),def(asin),
    def(atan),def(cosh),def(sinh),def(tanh),def(print),
    def(ceil),def(floor),def(round),def(trunc),
    def(T),def(t),def(dot),def(read),def(input),
    def(take),def(drop),def(filter),def(transform),
    def(enumerate),def(reverse),
    def(min),def(max),def(argmin),def(argmax),
    def(sec),def(csc),def(cot),def(exp),
    def(ln),def(lg),def(lb),
    def(rows),def(cols),
    def(arcsin),def(arccos),def(arctan),
  };
  #undef def
  std::sort(list.begin(),list.end());
  return list;
}

expr_t Nfn::eval_reserved_fn(){
  std::string_view id(name.begin()+1,name.end());
  constexpr auto fnlist=construct_reserved_fnlist();
  auto it=std::lower_bound(fnlist.begin(),fnlist.end(),id,[](const auto&a,const auto&b){ return a.first<b; });
  if(it==fnlist.end()||it->first!=id) throw std::runtime_error("未定義の予約語");
  try{
    return it->second(args);
  }catch(const std::runtime_error&e){
    this->error_exit(e.what());
    throw; // エラー対策
  }
}

json::value Nfn::to_json() const {
  json::value v;
  v["type"] = "fn";
  v["name"] = name;
  json::value::array_t arr;
  for (auto arg : args) {
    arr.push_back(arg ? arg->to_json() : json::value());
  }
  v["args"] = arr;
  v["row"] = (int64_t)row;
  v["col"] = (int64_t)col;
  return v;
}

} // namespace AST