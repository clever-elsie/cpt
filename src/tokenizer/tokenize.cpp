#include "tokenizer/tokenize.hpp"

tokenize::tokenize(std::string_view istr, std::string_view file_name)noexcept:istr(istr),row(1),col(1),file_name(file_name){
  prev_linehead=istr.begin();
  first=get_token();
}

std::string tokenize::gen_error_msg(const std::string_view&msg)const{
  // トークンの開始カラム位置（1-indexed）
  size_t tok_start_col = col > first.len ? col - first.len : 1;
  std::string ret = std::string(file_name) + ":" + std::to_string(row) + ":" + std::to_string(tok_start_col) + ": error: " + std::string(msg) + "\n";

  // prev_lineheadから行末までを取得
  auto line_start = prev_linehead;
  auto line_end = line_start;
  // istr の元のデータの終端を推定: istr.end() は残りの入力の終端
  // prev_linehead は istr の元のバッファ内なので、istr.end() を超えない範囲で走査
  // ただし istr が縮んでいる場合、prev_linehead の先は istr.end() の先にあることはない
  // (prev_linehead は常に istr.begin() 以前のポインタか、istr.begin() そのもの)
  // 安全のため: line_end を prev_linehead から istr.end() までの範囲で改行を探す
  auto buf_end = istr.end();
  // istr.begin() が prev_linehead より先なら buf_end を使える
  // prev_linehead が istr.begin() より前の場合、istr の元のバッファ内であることが保証される
  for(line_end = line_start; line_end != buf_end; ++line_end){
    if(*line_end == '\n' || *line_end == '\r') break;
  }

  std::string_view line_view(line_start, line_end);
  ret += std::string(line_view) + "\n";

  // キャレットの位置（0-indexed のインデント）
  size_t indent = tok_start_col > 0 ? tok_start_col - 1 : 0;
  for(size_t i = 0; i < indent; ++i)
    ret += " ";
  ret += "^";
  for(size_t i = 1; i < first.len; ++i)
    ret += "~";
  ret += "\n";
  return ret;
}

void tokenize::error_exit(const std::string_view&msg)noexcept(false){
  throw std::runtime_error(gen_error_msg(msg));
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
      if(*itr=='.'){
        if(itr+1 != istr.end() && *(itr+1) == '.'){
          goto ENDofLITERAL;
        }
        state=dot;
      }
      else if(c=='e') state=e;
      else if(c=='b') state=binb;
      else if(c=='x') state=hexx;
      else if(std::isdigit(*itr)) state=Digit;
      else goto ENDofLITERAL;
    }break;
    case Digit:{
      if(*itr=='.'){
        if(itr+1 != istr.end() && *(itr+1) == '.'){
          goto ENDofLITERAL;
        }
        state=dot;
      }
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
  bool is_complex = false;
  if(!istr.empty() && istr[0] == 'i'){
    is_complex = true;
    itr++;
    ret = std::string_view(ret.data(), ret.size() + 1);
    istr = std::string_view(itr, istr.end());
  }
  token_t t_type = token_t::DECIMAL;
  switch(state){
    case Zero:
    case Digit: t_type = is_complex ? token_t::COMPLEX : token_t::DECIMAL; break;
    case Hex: t_type = token_t::HEX; break;
    case Bin: t_type = token_t::BINARY; break;
    case Float:
    case Exp: t_type = is_complex ? token_t::COMPLEX : token_t::FLOAT; break;
    default:
      error_exit("数値の解析に失敗しました");
  }
  col+=ret.size();
  return{t_type, symbol_t::NAS, ret.size(), ret};
}

pToken tokenize::get_ident()noexcept{
  const token_t is_reserved = istr[0]=='\\'?token_t::RESERVED:token_t::IDENT;
  size_t i=1;
  for(;i<istr.size();++i) {
    if(is_reserved == token_t::RESERVED) {
      if(!std::isalnum(istr[i])) {
        if(istr[i] == '_' && i + 3 < istr.size() && istr.substr(i, 4) == "_mat") {
          i += 3;
        } else {
          break;
        }
      }
    } else {
      if(!std::isalnum(istr[i]) && istr[i] != '_') break;
    }
  }
  std::string_view ret(istr.begin(),i);
  istr=std::string_view(istr.begin()+i,istr.end());
  if(is_reserved==token_t::RESERVED){
    symbol_t sym=symbol_t::NAS;
    if(ret=="\\ge") sym=symbol_t::GE;
    else if(ret=="\\gt") sym=symbol_t::GT;
    else if(ret=="\\le") sym=symbol_t::LE;
    else if(ret=="\\lt") sym=symbol_t::LT;
    else if(ret=="\\land") sym=symbol_t::LAND;
    else if(ret=="\\lor") sym=symbol_t::LOR;
    else if(ret=="\\ne") sym=symbol_t::NE;
    if(sym!=symbol_t::NAS) return{token_t::RESERVED, sym, 0, std::string_view()};
  }
  col+=ret.size();
  return{is_reserved, symbol_t::NAS, ret.size(), ret};
}

/**
 * @brief 空白とコメントをスキップ
 * @return 文字列の終端のときtrue
 */
bool tokenize::skip_whitespace_and_comment()noexcept{
  bool is_comment;
  do{
    is_comment=false;
    auto itr=istr.begin();
    while(itr!=istr.end()&&std::isspace(*itr)){
      if(*itr=='\r'&&itr+1!=istr.end()&&*(itr+1)=='\n') ++itr;
      if(*itr=='\n'||*itr=='\r'){
        ++row;
        col=0; // ++itr,++colで1になる
        ++itr;
        ++col;
        prev_linehead=itr;
      }else{
        ++itr;
        ++col;
      }
    }
    if(is_comment=(itr!=istr.end()&&*itr=='#'))
      while(itr!=istr.end()&&(*itr!='\n'&&*itr!='\r')) ++itr;
    istr=std::string_view(itr,istr.end());
    if(itr==istr.end()) return true;
  }while(is_comment);
  return false;
}

pToken tokenize::get_token()noexcept{
  if(skip_whitespace_and_comment()) return {token_t::EMPTY, symbol_t::NAS, 0, std::string_view()};
  if(istr[0] == '"' || istr[0] == '\''){
    char quote = istr[0];
    size_t i = 1;
    while(i < istr.size() && istr[i] != quote){
      if(istr[i] == '\\' && i + 1 < istr.size()){
        i += 2;
      }else{
        i++;
      }
    }
    if(i >= istr.size()){
      error_exit("文字列リテラルが閉じられていません");
    }
    std::string_view ret(istr.begin(), i + 1);
    istr = std::string_view(istr.begin() + i + 1, istr.end());
    col += ret.size();
    return {token_t::STRING, symbol_t::NAS, ret.size(), ret};
  }
  if(std::isdigit(istr[0])) 
    return get_number();
  if(std::isalpha(istr[0])||istr[0]=='\\')
    return get_ident();
  symbol_t sym=symbol_t::NAS;
  size_t cnt=1;
  switch(istr[0]){ // caseは昇順．最適化を信じろ
    case '!':
      if(istr.size()>1&&istr[1]=='=') sym=symbol_t::NE, cnt=2;
      else sym=symbol_t::EXCL;
      break;
    case '%': sym=symbol_t::MOD; break;
    case '&': sym=symbol_t::LAND;
      if(istr.size()>1&&istr[0]==istr[1]) cnt=2;
      break;
    case '(': sym=symbol_t::LPAREN; break;
    case ')': sym=symbol_t::RPAREN; break;
    case '*':
      if(istr.size()>1&&istr[0]==istr[1]) sym=symbol_t::POW, cnt=2;
      else sym=symbol_t::MUL;
      break;
    case '+': sym=symbol_t::PLUS; break;
    case ',': sym=symbol_t::COMMA; break;
    case '-': sym=symbol_t::MINUS; break;
    case '.':
      if(istr.size()>1&&istr[1]=='.'){
        if(istr.size()>2&&istr[2]=='=') sym=symbol_t::DOTDOT_EQ, cnt=3;
        else sym=symbol_t::DOTDOT, cnt=2;
      }else{
        sym=symbol_t::DOT;
      }
      break;
    case '/':
      if(istr.size()>1&&istr[1]=='/') sym=symbol_t::IDIV, cnt=2;
      else sym=symbol_t::FDIV;
      break;
    case ':':
      if(istr.size()>1&&istr[1]=='=') sym=symbol_t::ASSIGN, cnt=2;
      else if(istr.size()>1&&istr[1]==':') sym=symbol_t::CC, cnt=2;
      else sym=symbol_t::COLON;
      break;
    case ';': sym=symbol_t::SEMICOLON; break;
    case '<':
      if(istr.size()>1&&istr[1]=='=') sym=symbol_t::LE, cnt=2;
      else sym=symbol_t::LT;
      break;
    case '=':
      if(istr.size()>1&&istr[1]=='=') sym=symbol_t::EEQ, cnt=2;
      else sym=symbol_t::EQ;
      break;
    case '>':
      if(istr.size()>1&&istr[1]=='=') sym=symbol_t::GE, cnt=2;
      else sym=symbol_t::GT;
      break;
    case '?': sym=symbol_t::QUEST; break;
    case '^': sym=symbol_t::CARET; break;
    case '_': sym=symbol_t::UNDERSCORE; break;
    case '[': sym=symbol_t::LSQUARE; break;
    case ']': sym=symbol_t::RSQUARE; break;
    case '{': sym=symbol_t::LCURLY; break;
    case '|':
      if(istr.size()>1&&istr[1]=='|') sym=symbol_t::LOR, cnt=2;
      else sym=symbol_t::PIPE, cnt=1;
      break;
    case '}': sym=symbol_t::RCURLY; break;
    default:
      error_exit(std::string(1, istr[0]) + " は無効な記号です");
      break;
  }
  col+=cnt;
  istr=istr.substr(cnt,istr.size());
  return {token_t::SYMBOL,sym, cnt, std::string_view()};
}