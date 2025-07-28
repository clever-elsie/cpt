#include "tokenize.hpp"
#include "expr/expr.hpp"
#include "top.hpp"
#include <string>
#include <cstddef>
#include <cctype>
#include <set>

namespace CALC{

bool define_fn(tokenize&tok);
expr_t define_var(tokenize&tok);
expr_t second(tokenize&tok,bool is_fn);

bool top(std::string_view istr){
  tokenize tok(istr);
  try{
    expr_t value=second(tok,false);
    std::cout<<std::boolalpha;
    std::visit([](const auto&x){
      std::cout<<x<<std::endl;
    },value);
  }catch(const except&e){ return false; }
  return true;
}

expr_t second(tokenize&tok,bool is_fn){
  expr_t hold;
  while(true){
    try{
      if(tok.top().token=="let") hold=define_var(tok);
      else if(tok.top().token=="def"){
        if(is_fn){
          std::cerr<<"関数の定義は関数内では行えません"<<std::endl;
          exit(EXIT_FAILURE);
        }
        if(!define_fn(tok)) exit(EXIT_FAILURE);
      }else hold=EXPR::expr(tok);
    }catch(const except&e){
      if(e==except::EMPTY) break;
      else throw e;
    }
    if(tok.top().token==";"||tok.top().token==",") tok.next_token();
  }
  return hold;
}

enum class ident_error{
  OK,
  EMPTY,
  RESERVED,
  FN,
};

ident_error check_ident(std::string_view ident){
  if(ident.size()==0) return ident_error::EMPTY;
  if(ident[0]=='\\') return ident_error::RESERVED;
  if(EXPR::fn_map.contains(ident)) return ident_error::FN;
  return ident_error::OK;
}

void error_ident(ident_error err,bool is_fn){
  switch(err){
    case ident_error::EMPTY:
      std::cerr<<"項が空文字列です"<<std::endl;
      break;
    case ident_error::RESERVED:
      std::cerr<<"予約語を使用する"<<(is_fn?"関数名":"変数名")<<"は使えません"<<std::endl;
      break;
    case ident_error::FN:
      if(is_fn) std::cerr<<"関数名が重複しています"<<std::endl;
      else std::cerr<<"関数名と重複する変数名は使えません"<<std::endl;
      break;
    default:
      std::cerr<<"不明なエラー"<<std::endl;
      break;
  }
  exit(EXIT_FAILURE);
}

expr_t define_var(tokenize&tok){
  tok.next_token();
  if(tok.top().type!=token_t::IDENT){
    std::cerr<<"変数名に使えない文字列が含まれてるかもね"<<std::endl;
    return false;
  }
  std::string_view variable=tok.top().token;
  if(check_ident(variable)!=ident_error::OK) error_ident(check_ident(variable),false);
  tok.next_token();
  if(tok.top().type!=token_t::SYMBOL||tok.top().token!="="){
    std::cerr<<"代入文では代入してください．"<<std::endl;
    return false;
  }
  tok.next_token();
  expr_t value=EXPR::expr(tok); // 例外は上へ
  if(auto itr=EXPR::var_map.find(variable);itr==EXPR::var_map.end())
    EXPR::var_map.emplace(variable,std::vector<expr_t>{std::move(value)});
  else itr->second.push_back(std::move(value));
  return true;
}

bool define_fn(tokenize&tok){
  tok.next_token();
  // def fn_name(arg1,...) { ... }
  // ブロック内部はパースせず，引数名と変数名のみ保存する
  // {でcnt++, }でcnt--，cnt==0で関数終了
  // 文法違反の時はUB
  if(tok.top().type!=token_t::IDENT){
    std::cerr<<"関数名に使えない文字列が含まれてるかもね"<<std::endl;
    return false;
  }
  std::string_view fn_name=tok.top().token;
  if(check_ident(fn_name)!=ident_error::OK) error_ident(check_ident(fn_name),true);
  std::set<std::string_view,CALC_MAP::StringEqual> Sargs,Svars;
  std::vector<std::string_view> args,vars;
  std::string body;
  tok.next_token();
  if(tok.top().type!=token_t::SYMBOL||tok.top().token!="("){
    std::cerr<<"関数の引数は()で囲んでください"<<std::endl;
    return false;
  }
  tok.next_token();
  while(tok.top().type==token_t::IDENT){
    if(check_ident(tok.top().token)!=ident_error::OK) error_ident(check_ident(tok.top().token),false);
    if(!Sargs.emplace(tok.top().token).second){
      std::cerr<<"関数"<<fn_name<<"の引数名が重複しています"<<std::endl;
      return false;
    }
    args.push_back(tok.top().token);
    tok.next_token();
    if(tok.top().type==token_t::SYMBOL&&tok.top().token==",") tok.next_token();
    else break;
  }
  if(tok.top().type!=token_t::SYMBOL||tok.top().token!=")"){
    std::cerr<<"関数の引数は()で囲んでください"<<std::endl;
    return false;
  }
  tok.next_token();
  if(tok.top().type!=token_t::SYMBOL||tok.top().token!="{"){
    std::cerr<<"関数の定義は{}で囲んでください"<<std::endl;
    return false;
  }
  tok.next_token();
  size_t cnt=1;
  bool is_var=false;
  while(true){
    if(tok.top().type==token_t::SYMBOL&&tok.top().token=="{") cnt++;
    else if(tok.top().type==token_t::SYMBOL&&tok.top().token=="}") cnt--;
    if(is_var){
      if(tok.top().type==token_t::IDENT){
        if(check_ident(tok.top().token)!=ident_error::OK) error_ident(check_ident(tok.top().token),false);
        if(!Svars.emplace(tok.top().token).second){
          std::cerr<<"関数"<<fn_name<<"の変数名が重複しています"<<std::endl;
          return false;
        }
        vars.push_back(tok.top().token);
      }else{
        std::cerr<<"変数名に使えない文字列が含まれてるかもね"<<std::endl;
        return false;
      }
      is_var=false;
    }
    if(tok.top().token=="let") is_var=true;
    if(cnt==0) break;
    body+=tok.top().token;
    body.push_back(' ');
    tok.next_token();
  }
  if(cnt!=0){
    std::cerr<<"関数"<<fn_name<<"の定義が閉じられていません"<<std::endl;
    return false;
  }else tok.next_token();
  EXPR::fn_map.emplace(fn_name,CALC_MAP::Fn(std::move(args),std::move(vars),std::move(body)));
  return true;
}
}// namespace CALC