#include "tokenize.hpp"

tokenize::tokenize(std::string_view istr)noexcept:istr(istr),row(1),col(1){
  first=get_token();
}
std::string tokenize::gen_error_msg(const std::string_view&msg)const{
  std::string ret=std::string(msg)+" : "+std::to_string(row)+":"+std::to_string(col)+"\n";
  auto itr=istr.begin();
  for(;itr!=istr.end();++itr)
    if(*itr=='\n'||*itr=='\r') break;
  std::string_view line_view(itr-col+1,itr);
  ret+=std::string(line_view)+"\n";
  for(size_t i=0;i<col-1;++i)
    ret+=" ";
  ret+="^\n";
  return ret;
}

void tokenize::error_exit(const std::string_view&msg)noexcept{
  std::cerr<<gen_error_msg(msg)<<std::endl;
  std::exit(EXIT_FAILURE);
}

void tokenize::error_throw(const std::string&msg)noexcept(false){
  throw std::runtime_error(gen_error_msg(msg));
}

std::pair<size_t,size_t> tokenize::get_pos()const noexcept{
  return {row,col};
}

pToken tokenize::next_token()noexcept{
  first=get_token();
  return first;
}

pToken tokenize::top()noexcept{
  return first;
}

pToken tokenize::get_number() noexcept {
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
      error_exit(__func__+std::string(" : 数値の解析に失敗しました"));
  }
  return{token_t::DECIMAL,ret};
}

pToken tokenize::get_ident()noexcept{
  const token_t is_reserverd = istr[0]=='\\'?token_t::RESERVED:token_t::IDENT;
  size_t i=1;
  for(;i<istr.size();++i)
    if(!std::isalnum(istr[i]))
      break;
  std::string_view ret(istr.begin(),i);
  istr=std::string_view(istr.begin()+i,istr.end());
  return{is_reserverd,ret};
}

pToken tokenize::get_token()noexcept{
  { // skip space and comment
    bool is_comment;
    do{
      is_comment=false;
      auto itr=istr.begin();
      while(itr!=istr.end()&&std::isspace(*itr)){
        if(*itr=='\r'&&itr+1!=istr.end()&&*(itr+1)=='\n') ++itr;
        if(*itr=='\n'||*itr=='\r') ++row,col=1;
        ++itr,++col;
      }
      if(is_comment=(itr!=istr.end()&&*itr=='#'))
        while(itr!=istr.end()&&(*itr!='\n'&&*itr!='\r')) ++itr;
      istr=std::string_view(itr,istr.end());
      if(itr==istr.end()) return {token_t::EMPTY,std::string_view()};
    }while(is_comment);
  }
  if(std::isdigit(istr[0])) 
    return get_number();
  if(std::isalpha(istr[0])||istr[0]=='\\')
    return get_ident();
  int cnt=1;
  switch(istr[0]){ // caseは昇順．最適化を信じろ
    case '!':
      if(istr.size()>1&&istr[1]=='=') cnt=2; break;
    case '%':break;
    case '&':
      if(istr.size()>1&&istr[0]==istr[1]) cnt=2; break;
    case '(': case ')': break;
    case '*':
      if(istr.size()>1&&istr[0]==istr[1]) cnt=2; break;
    case '+': case ',': case '-': break;
    case '/': if(istr.size()>1&&istr[1]=='/') cnt=2; break;
    case ':': if(istr.size()>1&&istr[1]=='=') cnt=2; break;
    case ';': break;
    case '<': case '=': case '>':
      if(istr.size()>1&&istr[1]=='=') cnt=2; break;
    case '?': case '^': case '_': case '{': break;
    case '|': if(istr.size()>1&&istr[1]=='|') cnt=2; break;
    case '}': break;
    default:
      error_exit(__func__+std::string(" : ")+istr[0]+"は無効な記号です");
      break;
  }
  std::string_view ret=istr.substr(0,cnt);
  istr=istr.substr(cnt,istr.size());
  return {token_t::SYMBOL,ret};
}