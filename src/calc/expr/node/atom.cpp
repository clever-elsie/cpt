#include "../node.hpp"
#include "reserved/reserved.hpp"
#include <array>
#include <algorithm>

namespace CALC{namespace EXPR{

expr_t atom(tokenize&tok) noexcept(false) {
  if(token_t::EMPTY==tok.top().type){
    std::cerr<<"項が空文字列です"<<std::endl;
    exit(EXIT_FAILURE);
  }else if(token_t::IDENT==tok.top().type){
    auto itr=var_map.find(tok.top().token);
    if(itr==var_map.end()||itr->second.size()==0){
      std::cerr<<"未定義変数"<<tok.top().token<<"を使ってるよ"<<std::endl;
      exit(EXIT_FAILURE);
    }
    tok.next_token();
    return itr->second.back();
  }else if(token_t::RESERVED==tok.top().type)
    return reserved_functions(tok);
  else if(tok.top().token=="!"){
    tok.next_token();
    expr_t value=atom(tok);
    if(std::holds_alternative<bool>(value)){
      return expr_t(!std::get<bool>(value));
    }else{
      std::cerr<<"無効な論理否定"<<std::endl;
      exit(EXIT_FAILURE);
    }
  }else if(tok.top().token=="("){
    tok.next_token();
    expr_t value=expr(tok);
    if(tok.top().token==")") tok.next_token();
    else{
      std::cerr<<"かっこが閉じられてないよ"<<std::endl;
      exit(EXIT_FAILURE);
    }
    return value;
  }else if(token_t::DECIMAL==tok.top().type) return parse_DECIMAL(tok);
  else if(token_t::FLOAT==tok.top().type) return parse_FLOAT(tok);
  else if(token_t::BINARY==tok.top().type) return parse_BINARY(tok);
  else if(token_t::HEX==tok.top().type) return parse_HEX(tok);
  else if(token_t::SYMBOL==tok.top().type){
    std::cerr<<tok.top().token<<"は無効な記号です"<<std::endl;
    exit(EXIT_FAILURE);
  }
  return expr_t(false);
}

using R_R=expr_t(*)(tokenize&)noexcept(false);
using psr=std::pair<std::string_view,R_R>;
constexpr auto construct_fn_list()noexcept(true){
#define r(name) {#name,&name}
  std::array<psr,15> fnlist{
    (psr)r(abs),
    r(sum), r(prod),
    r(log), r(log10),
    r(cos), r(sin), r(tan),
    r(acos), r(asin), r(atan),
    r(cosh), r(sinh), r(tanh),
    r(print)
  };
#undef r
  std::sort(fnlist.begin(),fnlist.end(),[](const psr&l,const psr&r){
    return l.first<r.first;
  });
  return fnlist;
}

expr_t reserved_functions(tokenize&tok)noexcept(false){
  std::string_view id=tok.top().token.substr(1);
  tok.next_token();
  const static constexpr auto fnlist=construct_fn_list();
  auto itr=std::lower_bound(fnlist.begin(),fnlist.end(),id,[](const psr&l,const std::string_view&r){
    return l.first<r;
  });
  if(itr==fnlist.end()||itr->first!=id){
    std::cerr<<id<<"は未定義関数です"<<std::endl;
    exit(EXIT_FAILURE);
  }
  return itr->second(tok);
}

}} // namespace EXPR}CALC}