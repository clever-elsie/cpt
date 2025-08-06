#include "../node.hpp"

namespace EXPR{

AST::Nitem* parse_DECIMAL(tokenize&tok){
  pToken tmp=tok.top();
  tok.next_token();
  auto [row,col]=tok.get_pos();
  return new AST::Nliteral(row,col,bint(tmp.token));
}

AST::Nitem* parse_BINARY(tokenize&tok){
  // 0b{0,1}+ -> HEX
  std::string_view token=tok.top().token;
  size_t rem=(token.size()-2)&0b11;
  size_t len=(rem>0)+((token.size()-2)>>2);
  size_t idx=2;
  std::string hex("0x");
  hex.reserve(len);
  if(rem){
    char p=0;
    if(rem==3&&token[idx++]=='1') p+=4;
    if(rem>=2&&token[idx++]=='1') p+=2;
    if(token[idx++]=='1') p++;
    hex.push_back(p<=9?'0'+p:'A'+p-10);
  }
  for(;idx<token.size();idx+=4){
    char p=(token[idx]-'0'<<3)+(token[idx+1]-'0'<<2)+(token[idx+2]-'0'<<1)+token[idx+3]-'0';
    hex.push_back(p<=9?'0'+p:'A'+p-10);
  }
  tok.next_token();
  auto [row,col]=tok.get_pos();
  return new AST::Nliteral(row,col,bint(hex));
}

AST::Nitem* parse_HEX(tokenize&tok){
  std::string_view number=tok.top().token;
  tok.next_token();
  auto [row,col]=tok.get_pos();
  return new AST::Nliteral(row,col,bint(number));
}

AST::Nitem* parse_FLOAT(tokenize&tok){
  pToken tmp=tok.top();
  tok.next_token();
  auto [row,col]=tok.get_pos();
  return new AST::Nliteral(row,col,bfloat(tmp.token));
}

} //namespace EXPR