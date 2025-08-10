#include "expr.hpp"

namespace EXPR{

AST::op_t get_op(symbol_t sym){ // 演算子省略乗算のためにそれ以上に弱い演算子を返す．強い方は処理済みなので不要．
  switch(sym){
    case symbol_t::PLUS: return AST::op_t::ADD;
    case symbol_t::MINUS: return AST::op_t::SUB;
    case symbol_t::MUL: return AST::op_t::MUL;
    case symbol_t::MOD: return AST::op_t::MOD;
    case symbol_t::FDIV: return AST::op_t::FDIV;
    case symbol_t::IDIV: return AST::op_t::IDIV;
    case symbol_t::LT: return AST::op_t::LT;
    case symbol_t::GT: return AST::op_t::GT;
    case symbol_t::LE: return AST::op_t::LE;
    case symbol_t::GE: return AST::op_t::GE;
    case symbol_t::EQ:
    case symbol_t::EEQ: return AST::op_t::EQ;
    case symbol_t::NE: return AST::op_t::NE;
    case symbol_t::LAND: return AST::op_t::LAND;
    case symbol_t::LOR: return AST::op_t::LOR;
    case symbol_t::QUEST:
    case symbol_t::COLON: return AST::op_t::BR;
    case symbol_t::ASSIGN: return AST::op_t::ASSIGN;
    default: break;
  }
  return AST::op_t::NOP;
}

AST::Nitem* or_expr(tokenize&tok) {
  std::vector<AST::Nitem*> items;
  items.push_back(and_expr(tok));
  while(tok.top().symbol==symbol_t::LOR){
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
  while(tok.top().symbol==symbol_t::LAND){
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
  std::vector<symbol_t> ops;
  items.push_back(algebra(tok));
  while(true){
    if(auto sym=tok.top().symbol;
      sym==symbol_t::LT||sym==symbol_t::GT||sym==symbol_t::LE||sym==symbol_t::GE
    ||sym==symbol_t::EQ||sym==symbol_t::EEQ||sym==symbol_t::NE)
      ops.push_back(sym);
    else break;
    tok.next_token();
    items.push_back(algebra(tok));
  }
  if(items.size()==1) return items.front();
  auto get_op=[](symbol_t sym)-> AST::op_t {
    switch(sym){
      case symbol_t::LT: return AST::op_t::LT;
      case symbol_t::GT: return AST::op_t::GT;
      case symbol_t::LE: return AST::op_t::LE;
      case symbol_t::GE: return AST::op_t::GE;
      case symbol_t::EQ:
      case symbol_t::EEQ: return AST::op_t::EQ;
      case symbol_t::NE: return AST::op_t::NE;
      default: throw std::runtime_error("無効な演算子");
    }
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
  while(tok.top().symbol==symbol_t::PLUS||tok.top().symbol==symbol_t::MINUS){
    AST::op_t op=get_op(tok.top().symbol);
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

bool is_multipliable(symbol_t sym){
  switch(sym){
    case symbol_t::SEMICOLON:
    case symbol_t::COMMA:
    case symbol_t::RPAREN:
    case symbol_t::RCURLY:
    case symbol_t::COLON:
    case symbol_t::QUEST: return false;
  }
  return true;
}

AST::Nitem* term(tokenize&tok) {
  std::vector<AST::Nitem*> items;
  std::vector<AST::op_t> ops;
  items.push_back(factor(tok));
  while(tok.top().type!=token_t::EMPTY){
    AST::op_t op=get_op(tok.top().symbol);
    if(op==AST::op_t::NOP&&is_multipliable(tok.top().symbol)){ // 演算子省略*
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
  if(auto sym=tok.top().symbol;sym==symbol_t::CARET||sym==symbol_t::POW){
    tok.next_token();
    AST::Nitem* rhs=factor(tok);
    auto [row,col]=lhs->get_pos();
    return new AST::Nexpr(row,col,AST::op_t::POW,lhs,rhs);
  }
  while(tok.top().symbol==symbol_t::EXCL){ // 階乗
    tok.next_token();
    auto [row,col]=lhs->get_pos();
    lhs=new AST::Nexpr(row,col,AST::op_t::FACT,lhs,nullptr);
  }
  return lhs;
}
}//namespace EXPR