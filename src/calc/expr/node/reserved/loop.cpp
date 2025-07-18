#include "reserved.hpp"
#include "misc.hpp"
namespace CALC{namespace EXPR{

template<auto op>
expr_t loop(tokenize&tok,expr_t&&ret)noexcept(false){
  std::optional<expr_t> e;
  std::optional<std::pair<std::string,expr_t>> b;
  for(int i=0;i<2;++i){
    if(tok.top().token=="^"){
      if(e.has_value()){
        std::cerr<<"上付きが多すぎます"<<std::endl;
        exit(EXIT_FAILURE);
      }
      e=get_idx(tok);
    }else if(tok.top().token=="_"){
      if(b.has_value()){
        std::cerr<<"下付きが多すぎます"<<std::endl;
        exit(EXIT_FAILURE);
      }
      b=get_below_with_declare(tok);
    }
  }
  if(!e.has_value()||!b.has_value()){
    if(!b.has_value()) std::cerr<<"開始条件がありません．"<<std::endl;
    if(!e.has_value()) std::cerr<<"終了条件がありません．"<<std::endl;
    exit(EXIT_FAILURE);
  }
  auto&itr=var_map[b.value().first].back();
  while(itr<=e.value()){
    expr_t value;
    if(itr==e.value()){
      if(tok.top().token=="(") value=expr(tok);
      else value=term(tok);
    }else{
      tokenize copy=tok;
      if(tok.top().token=="(") value=expr(copy);
      else value=term(copy);
    }
    op(ret,std::move(value));
    std::get<bint>(itr)+=1;
  }
  return ret;
}

expr_t sum(tokenize&tok)noexcept(false){
  return loop<[](expr_t&lhs,expr_t&&rhs){return lhs+=std::move(rhs);}>(tok,bint(0));
}
expr_t prod(tokenize&tok)noexcept(false){
  return loop<[](expr_t&lhs,expr_t&&rhs){return lhs*=std::move(rhs);}>(tok,bint(1));
}

}}//namespace EXPR}CALC}