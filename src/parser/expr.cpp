#include "expr.hpp"
#include "../tokenizer/tokenize.hpp"

namespace EXPR{

AST::Nitem* expr(tokenize&tok) {
  if(tok.top().type==token_t::EMPTY)
    throw except::EMPTY;
  AST::Nitem* ret=expr_3(tok);
  while(tok.top().token==":="){
    if(auto ptr=dynamic_cast<AST::Nvar*>(ret);ptr==nullptr)
      tok.error_throw("代入式の左辺は左辺値である必要があります．");
    tok.next_token();
    AST::Nitem* rhs=expr(tok);
    auto[row,col]=ret->get_pos();
    ret=new AST::Nexpr(row,col,AST::op_t::ASSIGN,ret,rhs);
  }
  return ret;
}

AST::Nitem* expr_3(tokenize&tok){
  AST::Nitem* e1=or_expr(tok);
  if(tok.top().type!=token_t::EMPTY && tok.top().token=="?"){
    tok.next_token(); // ?を消費
    AST::Nitem* e2=expr(tok);
    if(tok.top().type!=token_t::SYMBOL || tok.top().token!=":")
      tok.error_exit(__func__+std::string(" : 三項演算子は : で区切った2つの式を必要とします"));
    tok.next_token(); // :を消費
    AST::Nitem* e3=expr(tok);
    auto [row,col]=e1->get_pos();
    return new AST::Nexpr(row,col,AST::op_t::BR,e1,e2,e3);
  }
  return e1; // ?:ではない
}
} // namespace EXPR