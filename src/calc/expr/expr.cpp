#include "expr.hpp"
#include "../tokenize.hpp"
#include "node.hpp"

namespace CALC{ namespace EXPR{

expr_t expr(tokenize&tok) {
  if(tok.top().type==token_t::EMPTY)
    throw except::EMPTY;
  return expr_3(tok);
}

expr_t expr_3(tokenize&tok){
  expr_t e1=or_expr(tok);
  if(tok.top().type!=token_t::EMPTY && tok.top().token=="?"){
    if(!std::holds_alternative<bool>(e1)){
      if(std::holds_alternative<bint>(e1))
        e1=bint(static_cast<int>(std::get<bint>(e1))!=0);
      else if(std::holds_alternative<bfloat>(e1))
        e1=bfloat(std::get<bfloat>(e1)!=0);
      else tok.error_exit(__func__+std::string(" : 無効な条件式の型"));
    }
    tok.next_token(); // ?を消費
    expr_t e2=expr(tok);
    if(tok.top().type!=token_t::SYMBOL || tok.top().token!=":")
      tok.error_exit(__func__+std::string(" : 三項演算子は : で区切った2つの式を必要とします"));
    tok.next_token(); // :を消費
    expr_t e3=expr(tok);
    return std::get<bool>(e1)?e2:e3;
  }else return e1;
}
} } // namespace EXPR}CALC}