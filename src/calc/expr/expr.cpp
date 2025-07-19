#include "expr.hpp"
#include "../tokenize.hpp"
#include "node.hpp"

namespace CALC{ namespace EXPR{

expr_t expr(tokenize&tok) noexcept(false) {
  if(tok.top().type==token_t::EMPTY)
    throw except::EMPTY;
  return expr_3(tok);
}

expr_t expr_3(tokenize&tok) noexcept(false){
  expr_t e1=or_expr(tok);
  if(tok.top().type!=token_t::EMPTY && tok.top().token=="?"){
    if(!std::holds_alternative<bool>(e1)){
      std::cerr<<"3項演算子の条件はboolでなければなりません"<<std::endl;
      exit(EXIT_FAILURE);
    }
    tok.next_token(); // ?を消費
    expr_t e2=expr(tok);
    if(tok.top().type!=token_t::SYMBOL || tok.top().token!=":"){
      std::cerr<<"三項演算子は : で区切った2つの式を必要とします"<<std::endl;
      exit(EXIT_FAILURE);
    }
    tok.next_token(); // :を消費
    expr_t e3=expr(tok);
    return std::get<bool>(e1)?e2:e3;
  }else return e1;
}
}} // namespace EXPR}CALC}