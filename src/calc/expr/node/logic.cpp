#include "../expr.hpp"
#include "../../tokenize.hpp"
#include "../node.hpp"
namespace CALC{namespace EXPR{
expr_t or_expr(tokenize&tok) noexcept(false) {
  expr_t value=and_expr(tok);
  if(tok.top().type==token_t::SYMBOL
    && (tok.top().token=="||"||tok.top().token=="|")
  ){
    if(!std::holds_alternative<bool>(value)){
      std::cerr<<"論理和論理式にしか使えません"<<std::endl;
      exit(EXIT_FAILURE);
    }
    if(get<bool>(value))
      return expr_t(true);
  }else return value;
  while(tok.top().type==token_t::SYMBOL
    && (tok.top().token=="||"||tok.top().token=="|")
  ){
    tok.next_token();
    value=and_expr(tok);
    if(!std::holds_alternative<bool>(value)){
      std::cerr<<"論理和論理式にしか使えません"<<std::endl;
      exit(EXIT_FAILURE);
    }
    if(std::get<bool>(value))
      return expr_t(true);
  }
  return expr_t(false); 
}

expr_t and_expr(tokenize&tok) noexcept(false) {
  expr_t value=compare(tok);
  if(tok.top().type==token_t::SYMBOL && tok.top().token[0]=='&'){
    if(!std::holds_alternative<bool>(value)){
      std::cerr<<"論理積は論理式にしか使えません"<<std::endl;
      exit(EXIT_FAILURE);
    }
    if(!std::get<bool>(value))
      return expr_t(false);
  }else return value;
  while(tok.top().type==token_t::SYMBOL && tok.top().token[0]=='&'){
    tok.next_token();
    value=compare(tok);
    if(!std::holds_alternative<bool>(value)){
      std::cerr<<"論理積は論理式にしか使えません"<<std::endl;
      exit(EXIT_FAILURE);
    }
    if(!std::get<bool>(value))
      return expr_t(false);
  }
  return expr_t(true);
}

expr_t compare(tokenize&tok) noexcept(false) {
  expr_t lhs=algebra(tok);
  if(tok.top().type==token_t::SYMBOL
    &&(tok.top().token[0]=='<' || tok.top().token[0]=='>'
    || tok.top().token[0]=='=' || tok.top().token[0]=='!')
  ){
    while(tok.top().type==token_t::SYMBOL
      &&(tok.top().token[0]=='<' || tok.top().token[0]=='>'
      || tok.top().token[0]=='=' || tok.top().token=="!=")
    ){
      std::string_view op=tok.top().token;
      tok.next_token();
      expr_t rhs=algebra(tok);
      if(op[0]=='<'){
        if(op.size()==1){
          if(!(lhs<rhs)) return false;
        }else if(!(lhs<=rhs)) return false;
      }else if(op[0]=='>'){
        if(op.size()==1){
          if(!(lhs>rhs)) return false;
        }else if(!(lhs>=rhs)) return false;
      }else if(op[0]=='='){
        if(lhs!=rhs) return false;
      }else if(lhs==rhs) return false;
      lhs=std::move(rhs);
    }
    return expr_t(true);
  }
  return lhs;
}
}}//namespace EXPR}CALC}