#include "top.hpp"
#include "expr.hpp"
#include <string>
#include <cstddef>
#include <cctype>
#include <set>

namespace PARSER{

AST::Nstat* top(std::string_view istr){
  tokenize tok(istr);
  AST::Nstat*stat=new AST::Nstat();
  try{
    second(tok,false,stat);
  }catch(const except&e){
    delete stat;
    return nullptr;
  }
  return stat;
}

void second(tokenize&tok,bool is_fn,AST::Nstat*parent){
  while(true){
    try{
      if(tok.top().token=="let") parent->items.push_back(define_var(tok,parent));
      else if(tok.top().token=="def"){
        if(is_fn) tok.error_exit(__func__+std::string(" : 関数の定義は関数内では行えません"));
        define_fn(tok);
      }else parent->items.push_back(EXPR::expr(tok));
    }catch(const except&e){
      if(e==except::EMPTY) break;
      else throw e;
    }
    while(tok.top().token==";"||tok.top().token==",")
      tok.next_token();
    if(tok.top().token=="}") break;
  }
}

enum class ident_error{
  OK, EMPTY,
  RESERVED, FN,
};

ident_error check_ident(std::string_view ident){
  if(ident.size()==0) return ident_error::EMPTY;
  if(ident[0]=='\\') return ident_error::RESERVED;
  if(AST::fn_map.contains(ident)) return ident_error::FN;
  return ident_error::OK;
}

std::string error_ident(ident_error err,bool is_fn){
  switch(err){
    case ident_error::EMPTY:
      return "項が空文字列です";
    case ident_error::RESERVED:
      return "予約語を使用する"+std::string(is_fn?"関数名":"変数名")+"は使えません";
    case ident_error::FN:
      if(is_fn) return "関数名が重複しています";
      else return "関数名と重複する変数名は使えません";
    default: break;
  }
  return "不明なエラー";
}

// parentは\sumや\prodのときにはnullptr
AST::Ndecl* define_var(tokenize&tok,AST::Nstat*parent){
  tok.next_token();
  if(tok.top().type!=token_t::IDENT)
    tok.error_exit(__func__+std::string(" : 変数名に使えない文字列が含まれてるかもね"));
  std::string_view variable=tok.top().token;
  if(auto err=check_ident(variable);err!=ident_error::OK)
    tok.error_exit(__func__+std::string(" : ")+error_ident(err,false));
  tok.next_token();
  if(tok.top().type!=token_t::SYMBOL||(tok.top().token!="="&&tok.top().token!=":="))
    tok.error_exit(__func__+std::string(" : 宣言文では=か:=で代入してください．"));
  tok.next_token();
  AST::Nitem* expr=EXPR::expr(tok); // 例外は上へ
  if(parent&&parent->var_names_set.emplace(variable).second) // set追加に失敗したら戻り値はfalse
    parent->var_names.emplace_back(variable);
  return new AST::Ndecl(variable,expr,parent);
}

void define_fn(tokenize&tok){
  tok.next_token();
  // def fn_name(arg1,...) { ... }
  // ブロック内部はパースせず，引数名と変数名のみ保存する
  // {でcnt++, }でcnt--，cnt==0で関数終了
  // 文法違反の時はUB
  if(tok.top().type!=token_t::IDENT)
    tok.error_exit(__func__+std::string(" : 関数名に使えない文字列が含まれてるかもね"));
  std::string_view fn_name=tok.top().token;
  if(check_ident(fn_name)!=ident_error::OK) error_ident(check_ident(fn_name),true);
  std::unordered_set<std::string,MAP_VAR_FN::StringHash,MAP_VAR_FN::StringEqual> Sargs;
  std::vector<std::string> args;
  AST::Nstat*body=new AST::Nstat();
  tok.next_token();
  if(tok.top().type!=token_t::SYMBOL||tok.top().token!="(")
    tok.error_exit(__func__+std::string(" : 関数の引数は()で囲んでください"));
  tok.next_token();
  while(tok.top().type==token_t::IDENT){ // 引数
    if(check_ident(tok.top().token)!=ident_error::OK) error_ident(check_ident(tok.top().token),false);
    if(!Sargs.emplace(tok.top().token).second)
      tok.error_exit(__func__+std::string(" : 関数")+std::string(fn_name)+"の引数名が重複しています");
    args.emplace_back(tok.top().token);
    tok.next_token();
    if(tok.top().type==token_t::SYMBOL&&tok.top().token==",") tok.next_token();
    else break;
  }
  if(tok.top().type!=token_t::SYMBOL||tok.top().token!=")")
    tok.error_exit(__func__+std::string(" : 関数の引数は()で囲んでください"));
  tok.next_token();
  if(tok.top().type!=token_t::SYMBOL||tok.top().token!="{")
    tok.error_exit(__func__+std::string(" : 関数の定義は{}で囲んでください"));
  tok.next_token(); // {を消費
  second(tok,true,body);
  if(tok.top().token!="}") tok.error_exit(__func__+std::string(" : 関数")+std::string(fn_name)+"の定義が閉じられていません");
  tok.next_token(); // }を消費
  body->args=std::move(args);
  body->args_set=std::move(Sargs);
  AST::fn_map.emplace(fn_name,body);
}
}// namespace CALC