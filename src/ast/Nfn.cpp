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
  auto fn=fn_map.find(name);
  if(fn==fn_map.end()) throw std::runtime_error("未定義の関数");
  Nstat*fn_stat=fn->second;
  std::vector<expr_t>args;
  for(auto&&arg:this->args)
    args.push_back(arg->get_value());
  expr_t ret=fn_stat->evaluate(std::move(args));
  return ret;
}

constexpr std::array<std::pair<std::string_view,expr_t(*)(std::vector<Nitem*>&)>,19>
construct_reserved_fnlist(){
  using fn_t=expr_t(*)(std::vector<Nitem*>&);
  using fn_p_t=std::pair<std::string_view,fn_t>;
  #define def(name) fn_p_t{#name, name}
  std::array<fn_p_t,19> list{
    def(log),def(sum),def(prod),def(abs),def(log10),
    def(cos),def(sin),def(tan),def(acos),def(asin),
    def(atan),def(cosh),def(sinh),def(tanh),def(print),
    def(ceil),def(floor),def(round),def(trunc),
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
} // namespace AST