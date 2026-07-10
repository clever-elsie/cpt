#include "ast/reserved.hpp"
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
  fn_loop->items.clear();
  delete fn_loop;
  return init;
}

expr_t sum(std::vector<Nitem*>&args){
  return loop<[](expr_t&lhs,expr_t&&rhs){return lhs+rhs;}>(args,bint(0));
}
expr_t prod(std::vector<Nitem*>&args){
  return loop<[](expr_t&lhs,expr_t&&rhs){return lhs*rhs;}>(args,bint(1));
}

expr_t argmin(std::vector<Nitem*>&args){
  if(args.size()!=3) throw std::runtime_error("loop関数の引数不正");
  expr_t upper=args[0]->get_value();
  if(!upper.is<expr_t::types::BINT>()) throw std::runtime_error("ループの上限が整数ではありません");
  bint upper_value=upper.get<bint>();
  Nstat*fn_loop=new Nstat();
  auto decl=dynamic_cast<Ndecl*>(args[1]);
  if(decl==nullptr) throw std::runtime_error("ループ変数の宣言が見つかりません");
  decl->get_value();
  auto itr=var_map.find(decl->get_name());
  if(itr==var_map.end()) throw std::runtime_error("ループ変数が見つかりません");
  auto&itr_expr_t=itr->second.back();
  if(!itr_expr_t.is<expr_t::types::BINT>()) throw std::runtime_error("ループ変数が整数ではありません");
  bint&itr_value=itr_expr_t.get<bint>();
  fn_loop->items.push_back(args[2]);
  
  bool first = true;
  expr_t best_i = bint(0);
  expr_t best_val;
  
  for(;itr_value<=upper_value;++itr_value){
    expr_t current_i = itr_value;
    expr_t val = fn_loop->evaluate({});
    if(first){
      best_val = val;
      best_i = current_i;
      first = false;
    } else {
      if(val < best_val){
        best_val = val;
        best_i = current_i;
      }
    }
  }
  itr->second.pop_back();
  fn_loop->items.clear();
  delete fn_loop;
  return best_i;
}

expr_t argmax(std::vector<Nitem*>&args){
  if(args.size()!=3) throw std::runtime_error("loop関数の引数不正");
  expr_t upper=args[0]->get_value();
  if(!upper.is<expr_t::types::BINT>()) throw std::runtime_error("ループの上限が整数ではありません");
  bint upper_value=upper.get<bint>();
  Nstat*fn_loop=new Nstat();
  auto decl=dynamic_cast<Ndecl*>(args[1]);
  if(decl==nullptr) throw std::runtime_error("ループ変数の宣言が見つかりません");
  decl->get_value();
  auto itr=var_map.find(decl->get_name());
  if(itr==var_map.end()) throw std::runtime_error("ループ変数が見つかりません");
  auto&itr_expr_t=itr->second.back();
  if(!itr_expr_t.is<expr_t::types::BINT>()) throw std::runtime_error("ループ変数が整数ではありません");
  bint&itr_value=itr_expr_t.get<bint>();
  fn_loop->items.push_back(args[2]);
  
  bool first = true;
  expr_t best_i = bint(0);
  expr_t best_val;
  
  for(;itr_value<=upper_value;++itr_value){
    expr_t current_i = itr_value;
    expr_t val = fn_loop->evaluate({});
    if(first){
      best_val = val;
      best_i = current_i;
      first = false;
    } else {
      if(best_val < val){
        best_val = val;
        best_i = current_i;
      }
    }
  }
  itr->second.pop_back();
  fn_loop->items.clear();
  delete fn_loop;
  return best_i;
}

}//namespace EXPR