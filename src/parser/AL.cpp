#include "expr.hpp"

namespace EXPR{

AST::Nitem* or_expr(tokenize&tok) {
  std::vector<AST::Nitem*> items;
  items.push_back(and_expr(tok));
  while(tok.top().type==token_t::SYMBOL
    && (tok.top().token=="||"||tok.top().token=="|")
  ){
    tok.next_token();
    items.push_back(and_expr(tok));
  }
  while(items.size()>1){
    auto rhs=items.back();
    items.pop_back();
    auto lhs=items.back();
    items.pop_back();
    auto [row,col]=lhs->get_pos();
    items.push_back(new AST::Nexpr(row,col,AST::op_t::LOR,lhs,rhs));
  }
  return items.front();
}

AST::Nitem* and_expr(tokenize&tok) {
  std::vector<AST::Nitem*> items;
  items.push_back(compare(tok));
  while(tok.top().type==token_t::SYMBOL
    && (tok.top().token=="&&" || tok.top().token=="&")
  ){
    tok.next_token();
    items.push_back(compare(tok));
  }
  while(items.size()>1){
    auto rhs=items.back();
    items.pop_back();
    auto lhs=items.back();
    items.pop_back();
    auto [row,col]=lhs->get_pos();
    items.push_back(new AST::Nexpr(row,col,AST::op_t::LAND,lhs,rhs));
  }
  return items.front();
}

AST::Nitem* compare(tokenize&tok) {
  std::vector<AST::Nitem*> items;
  std::vector<std::string_view> ops;
  AST::Nitem* lhs=algebra(tok);
  while(true){
    if(tok.top().token=="<"||tok.top().token==">"||tok.top().token=="<="||tok.top().token==">="
    ||tok.top().token=="="||tok.top().token=="=="||tok.top().token=="!=")
      ops.push_back(tok.top().token);
    else break;
    items.push_back(algebra(tok));
  }
  if(items.size()==1) return items.front();
  auto get_op=[](std::string_view op)-> AST::op_t {
    if(op=="<") return AST::op_t::LT;
    if(op==">") return AST::op_t::GT;
    if(op=="<=") return AST::op_t::LE;
    if(op==">=") return AST::op_t::GE;
    if(op=="=") return AST::op_t::EQ;
    if(op=="==") return AST::op_t::EQ;
    if(op=="!=") return AST::op_t::NE;
    throw std::runtime_error("無効な演算子");
  };
  AST::Nitem* front=nullptr;
  for(ssize_t i=ops.size()-2;i>=0;--i){
    auto [row,col]=items[i]->get_pos();
    front=new AST::Nexpr(row,col,get_op(ops[i]),i==0?items[i]:nullptr,items[i+1],front);
  }
  return front;
}

AST::Nitem* algebra(tokenize&tok) {
  auto get_op=[](std::string_view op)->AST::op_t {
    if(op=="+") return AST::op_t::ADD;
    if(op=="-") return AST::op_t::SUB;
    throw std::runtime_error("無効な演算子");
  };
  std::vector<AST::Nitem*> items;
  std::vector<AST::op_t> ops;
  items.push_back(term(tok));
  while(tok.top().type==token_t::SYMBOL
    &&(tok.top().token=="+" || tok.top().token=="-")
  ){
    AST::op_t op=get_op(tok.top().token);
    tok.next_token();
    items.push_back(term(tok));
    ops.push_back(op);
  }
  AST::Nitem* front=items.front();
  for(size_t i=1;i<items.size();++i){
    auto [row,col]=items[i]->get_pos();
    front=new AST::Nexpr(row,col,ops[i-1],front,items[i]);
  }
  return front;
}

AST::Nitem* term(tokenize&tok) {
  auto get_op=[](std::string_view op)->AST::op_t {
    if(op=="*") return AST::op_t::MUL;
    if(op=="%") return AST::op_t::MOD;
    if(op=="/") return AST::op_t::FDIV;
    if(op=="//") return AST::op_t::IDIV;
    return AST::op_t::NOP;
  };
  std::vector<AST::Nitem*> items;
  std::vector<AST::op_t> ops;
  items.push_back(factor(tok));
  while(tok.top().type!=token_t::EMPTY){
    AST::op_t op=get_op(tok.top().token);
    if(op==AST::op_t::NOP){ // 演算子省略*を試す
      try{
        tokenize tok2=tok;
        items.push_back(factor(tok2)); // factor以下はerror_exitではなくerror_throw
        ops.push_back(op);
        tok=tok2; // 成功時はtokを進める
      }catch(const std::runtime_error&e){ break; } // 失敗時は無かったことにする
    }else{
      tok.next_token();
      items.push_back(factor(tok));
      ops.push_back(op);
    }
  }
  AST::Nitem* front=items.front();
  for(size_t i=1;i<items.size();++i){
    auto [row,col]=items[i]->get_pos();
    front=new AST::Nexpr(row,col,ops[i-1],front,items[i]);
  }
  return front;
}

AST::Nitem* factor(tokenize&tok) {
  AST::Nitem* lhs=atom(tok);
  if(tok.top().token=="^"||tok.top().token=="**"){
    tok.next_token();
    AST::Nitem* rhs=factor(tok);
    auto [row,col]=lhs->get_pos();
    return new AST::Nexpr(row,col,AST::op_t::POW,lhs,rhs);
  }
  while(tok.top().token=="!"){ // 階乗
    tok.next_token();
    auto [row,col]=lhs->get_pos();
    lhs=new AST::Nexpr(row,col,AST::op_t::FACT,lhs,nullptr);
  }
  return lhs;
}
}//namespace EXPR