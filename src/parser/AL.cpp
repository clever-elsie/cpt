#include "expr.hpp"

namespace EXPR{

AST::op_t get_op(std::string_view op){ // 演算子省略乗算のためにそれ以上に弱い演算子を返す．強い方は処理済みなので不要．
  if(op=="+") return AST::op_t::ADD;
  if(op=="-") return AST::op_t::SUB;
  if(op=="*") return AST::op_t::MUL;
  if(op=="%") return AST::op_t::MOD;
  if(op=="/") return AST::op_t::FDIV;
  if(op=="//") return AST::op_t::IDIV;
  if(op=="<") return AST::op_t::LT;
  if(op==">") return AST::op_t::GT;
  if(op=="<=") return AST::op_t::LE;
  if(op==">=") return AST::op_t::GE;
  if(op=="="||op=="==") return AST::op_t::EQ;
  if(op=="!=") return AST::op_t::NE;
  if(op=="&&"||op=="&") return AST::op_t::LAND;
  if(op=="||"||op=="|") return AST::op_t::LOR;
  if(op=="?"||op==":") return AST::op_t::BR;
  if(op==":=") return AST::op_t::ASSIGN;
  return AST::op_t::NOP;
}

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
  items.push_back(algebra(tok));
  while(true){
    if(tok.top().token=="<"||tok.top().token==">"||tok.top().token=="<="||tok.top().token==">="
    ||tok.top().token=="="||tok.top().token=="=="||tok.top().token=="!=")
      ops.push_back(tok.top().token);
    else break;
    tok.next_token();
    items.push_back(algebra(tok));
  }
  if(items.size()==1) return items.front();
  auto get_op=[](std::string_view op)-> AST::op_t {
    if(op=="<") return AST::op_t::LT;
    if(op==">") return AST::op_t::GT;
    if(op=="<=") return AST::op_t::LE;
    if(op==">=") return AST::op_t::GE;
    if(op=="="||op=="==") return AST::op_t::EQ;
    if(op=="!=") return AST::op_t::NE;
    throw std::runtime_error("無効な演算子");
  };
  AST::Nitem* front=nullptr;
  for(ssize_t i=items.size()-2;i>=0;--i){
    auto [row,col]=items[i]->get_pos();
    front=new AST::Nexpr(row,col,get_op(ops[i]),i==0?items[i]:nullptr,items[i+1],front);
  }
  return front;
}

AST::Nitem* algebra(tokenize&tok) {
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

bool is_multipliable(std::string_view op){
  if(op==";"||op==","||op==")"||op=="}"||op==":"||op=="?") return false;
  return true;
}

AST::Nitem* term(tokenize&tok) {
  std::vector<AST::Nitem*> items;
  std::vector<AST::op_t> ops;
  items.push_back(factor(tok));
  while(tok.top().type!=token_t::EMPTY){
    AST::op_t op=get_op(tok.top().token);
    if(op==AST::op_t::NOP&&is_multipliable(tok.top().token)){ // 演算子省略*
      items.push_back(factor(tok));
      ops.push_back(AST::op_t::MUL);
    }else if(op==AST::op_t::MUL||op==AST::op_t::MOD||op==AST::op_t::FDIV||op==AST::op_t::IDIV){
      tok.next_token();
      items.push_back(factor(tok));
      ops.push_back(op);
    }else break;
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