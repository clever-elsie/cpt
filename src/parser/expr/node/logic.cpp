#include "../expr.hpp"
#include "../../../tokenizer/tokenize.hpp"
#include "../node.hpp"

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
}//namespace EXPR