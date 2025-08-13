#include "parser/expr.hpp"
#include "parser/top.hpp"
#include <array>
#include <algorithm>
#include <ranges>

namespace EXPR{

std::vector<AST::Nitem*> get_args(tokenize&tok){
  std::vector<AST::Nitem*> args;
  if(tok.top().symbol!=symbol_t::LPAREN) tok.error_throw(__func__+std::string(" : 関数呼び出しの引数が(で囲まれていません"));
  tok.next_token(); // (を消費
  while(true){
    args.push_back(expr(tok));
    if(tok.top().symbol==symbol_t::COMMA) tok.next_token();
    else if(tok.top().symbol==symbol_t::RPAREN) break;
    else tok.error_throw(__func__+std::string(" : 関数呼び出しの引数が,で区切られていません"));
  }
  if(tok.top().symbol!=symbol_t::RPAREN) tok.error_throw(__func__+std::string(" : 関数呼び出しの引数が)で囲まれていません"));
  tok.next_token();
  return args;
}

template<bool is_below_declare>
std::pair<AST::Nitem*,AST::Nitem*> get_right_args(tokenize&tok){ // {上付き，下付き}
  AST::Nitem* exp=nullptr, *below=nullptr;
  auto get_arg=[&tok](AST::Nitem*&arg,bool is_below)->void {
    tok.next_token();
    if(tok.top().symbol==symbol_t::LCURLY){
      if(is_below&&is_below_declare)
        arg=PARSER::define_var(tok,nullptr);
      else{
        tok.next_token();
        arg=expr(tok);
      }
      if(tok.top().symbol!=symbol_t::RCURLY) tok.error_throw(__func__+std::string(" : {}が閉じられていません"));
      tok.next_token();
    }else{
      if(is_below&&is_below_declare) tok.error_throw(__func__+std::string(" : 下付きの引数が宣言されていません"));
      arg=atom(tok);
    }
  };
  while(exp==nullptr||below==nullptr){
    if(tok.top().symbol==symbol_t::CARET) get_arg(exp,false);
    else if(tok.top().symbol==symbol_t::UNDERSCORE) get_arg(below,true);
    else break;
  }
  return{exp,below};
}

AST::Nitem* function_call(tokenize&tok){
  std::string_view name=tok.top().token;
  tok.next_token(); // 関数名を消費
  // 上付きべき乗があるときはpowのノードを作成する
  auto [row,col]=tok.get_pos();
  auto [exp,below]=get_right_args<false>(tok);
  AST::Nfn* ret=new AST::Nfn(row,col,name,get_args(tok));
  if(below!=nullptr) throw std::runtime_error("関数"+std::string(name)+"に下付きの引数はありません");
  if(exp!=nullptr) return new AST::Nexpr(row,col,AST::op_t::POW,ret,exp);
  return ret;
}

AST::Nitem* reserved_function_call(tokenize&tok){
  std::string_view name=tok.top().token;
  // 上付きと下付きが両方あるとき，\sumや\prodでは必ず，上付き，下付きの順番に第一引数，第二引数に入る
  // 下付きだけのlogは第一引数に底を入れる．もし底がないとき，自然対数の底eを底とし，第一引数はnullptrとする．
  if(name=="\\log"){ // 下付き特殊
    tok.next_token();
    auto [row,col]=tok.get_pos();
    auto[exp,base]=get_right_args<false>(tok);
    std::vector<AST::Nitem*> arg=get_args(tok);
    std::vector<AST::Nitem*>args{base};
    args.insert(args.end(),arg.begin(),arg.end());
    AST::Nitem* ret=new AST::Nfn(row,col,name,std::move(args));
    if(exp!=nullptr) return new AST::Nexpr(row,col,AST::op_t::POW,ret,exp);
    return ret;
  }else if(name=="\\sum"||name=="\\prod"){ // 上下付き特殊
    tok.next_token();
    auto [row,col]=tok.get_pos();
    auto[exp,base]=get_right_args<true>(tok);
    std::vector<AST::Nitem*>args{exp,base};
    args.push_back(expr(tok));
    return new AST::Nfn(row,col,name,std::move(args));
  }else return function_call(tok); // 普通の関数
}

template<size_t base>
AST::Nitem* parse_literal(tokenize&tok){
  if constexpr(base==2){
    // 0b{0,1}+ -> HEX
    std::string_view token=tok.top().token;
    auto [row,col]=tok.get_pos();
    tok.next_token();
    size_t rem=(token.size()-2)&0b11;
    size_t len=(rem>0)+((token.size()-2)>>2);
    size_t idx=2;
    std::string hex("0x");
    hex.reserve(len);
    if(char p=0;rem){
      if(rem==3&&token[idx++]=='1') p+=4;
      if(rem>=2&&token[idx++]=='1') p+=2;
      if(token[idx++]=='1') p++;
      hex.push_back(p<=9?'0'+p:'A'+p-10);
    }
    for(;idx<token.size();idx+=4){
      char p=(token[idx]-'0'<<3)+(token[idx+1]-'0'<<2)+(token[idx+2]-'0'<<1)+token[idx+3]-'0';
      hex.push_back(p<=9?'0'+p:'A'+p-10);
    }
    return new AST::Nliteral(row,col,bint(hex));
  }else{
    pToken tmp=tok.top();
    tok.next_token();
    auto [row,col]=tok.get_pos();
    if constexpr(base==0) // float
      return new AST::Nliteral(row,col,bfloat(tmp.token));
    else if constexpr(base==10||base==16)
      return new AST::Nliteral(row,col,bint(tmp.token));
  }
}

AST::Nitem* atom(tokenize&tok) {
  if(token_t::EMPTY==tok.top().type)
    tok.error_throw(__func__+std::string(" : 項が空文字列です"));
  else if(token_t::IDENT==tok.top().type){
    if(auto ftr=AST::fn_map.find(tok.top().token);ftr!=AST::fn_map.end()) // 関数
      return function_call(tok);
    // 変数
    auto [row,col]=tok.get_pos();
    AST::Nitem* ret=new AST::Nvar(row,col,tok.top().token);
    tok.next_token(); // 変数名を消費
    return ret;
  }else if(token_t::RESERVED==tok.top().type) return reserved_function_call(tok);
  else if(tok.top().symbol==symbol_t::EXCL){
    auto [row,col]=tok.get_pos();
    tok.next_token(); // !を消費
    return new AST::Nexpr(row,col,AST::op_t::NOT,atom(tok),nullptr);
  }else if(tok.top().symbol==symbol_t::MINUS){
    auto [row,col]=tok.get_pos();
    tok.next_token(); // -を消費
    return new AST::Nexpr(row,col,AST::op_t::NEG,atom(tok),nullptr);
  }else if(tok.top().symbol==symbol_t::LPAREN){
    tok.next_token();
    AST::Nitem* ret=expr(tok);
    if(tok.top().symbol!=symbol_t::RPAREN) tok.error_throw(__func__+std::string(" : かっこが閉じられてないよ"));
    tok.next_token();
    return ret;
  }else if(token_t::DECIMAL==tok.top().type) return parse_literal<10>(tok);
  else if(token_t::FLOAT==tok.top().type) return parse_literal<0>(tok);
  else if(token_t::BINARY==tok.top().type) return parse_literal<2>(tok);
  else if(token_t::HEX==tok.top().type) return parse_literal<16>(tok);
  else if(token_t::SYMBOL==tok.top().type)
    tok.error_throw(__func__+std::string(" : ")+std::string(tok.top().token)+"は無効な記号です");
  return nullptr;
}

} //namespace EXPR