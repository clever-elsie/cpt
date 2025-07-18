#include "tokenize.hpp"
#include "expr/expr.hpp"
namespace CALC{

tokenize::tokenize(std::string_view&istr)noexcept(false):istr(istr){
  first=get_token();
}

pToken tokenize::next_token()noexcept(false){
  first=get_token();
  return first;
}

pToken tokenize::top()noexcept(false){
  return first;
}

pToken tokenize::get_number() noexcept(false) {
  // \d+, \d+., \d+.\d+, .\d+
  // 0x[\dA-Fa-f]+
  // 0b[01]+
  // -> e[+-]\d+
  enum State{ // 大文字スタートは終止状態
    start, dot, Zero, Digit,
    hexx, Hex, binb, Bin,
    Float, e, epm,Exp
  };
  State state=start;
  auto itr=istr.begin();
  for(;itr!=istr.end();++itr)
  switch(state){
    case start:{
      if(*itr=='0') state=Zero;
      else if(std::isdigit(*itr)) state=Digit;
      else goto ENDofLITERAL;
    }break;
    case dot:{
      if(*itr=='e'||*itr=='E') state=e;
      else if(std::isdigit(*itr)) state=Float;
      else goto ENDofLITERAL;
    }break;
    case Zero:{
      char c=std::tolower(*itr);
      if(*itr=='.') state=dot;
      else if(c=='e') state=e;
      else if(c=='b') state=binb;
      else if(c=='x') state=hexx;
      else if(std::isdigit(*itr)) state=Digit;
      else goto ENDofLITERAL;
    }break;
    case Digit:{
      if(*itr=='.') state=dot;
      else if(std::tolower(*itr)=='e') state=e;
      else if(!std::isdigit(*itr)) goto ENDofLITERAL;
    }break;
    case hexx:{
      char c=std::tolower(*itr);
      if(std::isdigit(*itr)||('a'<=c&&c<='f')) state=Hex;
      else goto ENDofLITERAL;
    }break;
    case Hex:{
      char c=std::tolower(*itr);
      if(!std::isdigit(*itr)&&(c<'a'||'f'<c))
        goto ENDofLITERAL;
    }break;
    case binb:{
      if(*itr=='0'||*itr=='1') state=Bin;
      else goto ENDofLITERAL;
    }break;
    case Bin:{
      if(*itr<'0'||'1'<*itr) goto ENDofLITERAL;
    }break;
    case Float:{
      if(std::tolower(*itr)=='e') state=e;
      else if(!std::isdigit(*itr)) goto ENDofLITERAL;
    }break;
    case e:{
      if(std::isdigit(*itr)) state=Exp;
      else if(*itr=='+'||*itr=='-') state=epm;
      else goto ENDofLITERAL;
    }break;
    case epm:{
      if(std::isdigit(*itr)) state=Exp;
      else goto ENDofLITERAL;
    }break;
    case Exp:{
      if(!std::isdigit(*itr))
        goto ENDofLITERAL;
    }break;
  }
ENDofLITERAL:
  std::string_view ret(istr.begin(),itr);
  istr=std::string_view(itr,istr.end());
  switch(state){
    case Zero:
    case Digit:return{token_t::DECIMAL,ret}; break;
    case Hex: return{token_t::HEX, ret}; break;
    case Bin: return{token_t::BINARY,ret}; break;
    case Float: return{token_t::FLOAT, ret}; break;
    case Exp: return{token_t::FLOAT, ret}; break;
    default:
      throw except::INVALID_TOKEN;
  }
  return{token_t::DECIMAL,ret};
}

pToken tokenize::get_ident()noexcept(true){
  const token_t is_reserverd = istr[0]=='\\'?token_t::RESERVED:token_t::IDENT;
  size_t i=1;
  for(;i<istr.size();++i)
    if(!std::isalnum(istr[i])&&!istr[i]=='_')
      break;
  std::string_view ret(istr.begin(),i);
  ret=std::string_view(istr.begin()+i,istr.end());
  return{is_reserverd,ret};
}

pToken tokenize::get_token()noexcept(false){
  { // space skip
    auto itr=istr.begin();
    while(itr!=istr.end()&&std::isspace(*itr)) ++itr;
    if(itr==istr.end()) return {token_t::EMPTY,std::string_view()};
    istr=std::string_view(itr,istr.end());
  }
  if(std::isdigit(istr[0])) 
    return get_number();
  if(std::isalpha(istr[0])||istr[0]=='\\'||istr[0]=='_')
    return get_ident();
  int cnt=1;
  switch(istr[0]){ // caseは昇順．最適化を信じろ
    case '!': case '%':break;
    case '&': case '*': if(istr.size()>1&&istr[0]==istr[1]) cnt=2; break;
    case '+': case ',': case '-': break;
    case '/': if(istr.size()>1&&istr[1]=='/') cnt=2; break;
    case ';': break;
    case '<': case '=': case '>':
      if(istr.size()>1&&istr[1]=='=') cnt=2; break;
    case '^': case '_': case '{': break;
    case '|': if(istr.size()>1&&istr[1]=='|') cnt=2; break;
    case '}': break;
    default:
      std::cerr<<static_cast<int>(istr[0])<<"は無効な記号です"<<std::endl;
      exit(EXIT_FAILURE);
      break;
  }
  std::string_view ret=istr.substr(0,cnt);
  istr=istr.substr(cnt,istr.size());
  return {token_t::SYMBOL,ret};
}
}//namespace CALC