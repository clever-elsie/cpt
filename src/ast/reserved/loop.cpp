#include "reserved.hpp"
#include <type_traits>

namespace AST{

template<auto op>
expr_t loop(std::vector<Nitem*>&args,expr_t&&init){
  if(args.size()!=3) throw std::runtime_error("ループ関数の引数不正");
  expr_t upper=args[0]->get_value();
  if(!upper.is<expr_t::types::BINT>()) throw std::runtime_error("ループの上限が整数ではありません");
  bint upper_value=upper.get<bint>();
  Nstat*fn_loop=new Nstat();
  auto decl=dynamic_cast<Ndecl*>(args[1]);
  if(decl==nullptr) throw std::runtime_error("ループ変数の宣言が見つかりません");
  decl->get_value(); // var_mapにループ変数を登録
  auto itr=var_map.find(decl->get_name());
  if(itr==var_map.end()) throw std::runtime_error("ループ変数が見つかりません");
  auto&itr_expr_t=itr->second.back(); // ループ変数の値
  if(!itr_expr_t.is<expr_t::types::BINT>()) throw std::runtime_error("ループ変数が整数ではありません");
  bint&itr_value=itr_expr_t.get<bint>();
  fn_loop->items.push_back(args[2]); // ループ内式をloop関数に移動
  for(;itr_value<=upper_value;++itr_value)
    init=op(init,fn_loop->evaluate({})); // ループ内式を評価
  itr->second.pop_back();
  return init;
}

expr_t sum(std::vector<Nitem*>&args){
  return loop<[](expr_t&lhs,expr_t&&rhs){return lhs+rhs;}>(args,bint(0));
}
expr_t prod(std::vector<Nitem*>&args){
  return loop<[](expr_t&lhs,expr_t&&rhs){return lhs*rhs;}>(args,bint(1));
}

}//namespace EXPR