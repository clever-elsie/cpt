#include <fstream>
#include "parser/top.hpp"
#include "parser/expr.hpp"
#include <string>
#include <cstddef>
#include <cctype>
#include <set>
#include <boost/math/constants/constants.hpp>
#include <limits>

namespace PARSER{

void init_constants() {
  static bool initialized = false;
  if(initialized) return;
  initialized = true;

  AST::var_map["\\pi"] = { expr_t(boost::math::constants::pi<bfloat>()) };
  AST::var_map["\\e"] = { expr_t(boost::math::constants::e<bfloat>()) };
  AST::var_map["\\i"] = { expr_t(bcomplex(0, 1)) };
  AST::var_map["\\inf"] = { expr_t(std::numeric_limits<bfloat>::infinity()) };
  AST::var_map["\\nan"] = { expr_t(std::numeric_limits<bfloat>::quiet_NaN()) };
  AST::var_map["\\eps"] = { expr_t(std::numeric_limits<bfloat>::epsilon()) };
  AST::var_map["true"] = { expr_t(true) };
  AST::var_map["false"] = { expr_t(false) };
}

AST::Nstat* top(std::string_view istr){
  init_constants();
  tokenize tok(istr);
  AST::Nstat*stat=new AST::Nstat();
  try{
    second(tok,false,stat);
  }catch(const except&e){
    delete stat;
    return nullptr;
  }
  AST::input_buffer = std::string(tok.get_remaining());
  return stat;
}

void second(tokenize&tok,bool is_fn,AST::Nstat*parent,std::string_view ns_prefix){
  auto skip_sep=[](tokenize&tok){
    while(tok.top().symbol==symbol_t::SEMICOLON
        ||tok.top().symbol==symbol_t::COMMA)
      tok.next_token();
  };
  while(true){
    skip_sep(tok);
    try{
      if(tok.top().token=="let") parent->items.push_back(define_var(tok,parent,ns_prefix));
      else if(tok.top().token=="exit"){
        break;
      }
      else if(tok.top().token=="def"){
        if(is_fn) tok.error_exit(__func__+std::string(" : 関数の定義は関数内では行えません"));
        define_fn(tok,ns_prefix);
      }else if(tok.top().token=="import"){
        tok.next_token();
        if(tok.top().type != token_t::STRING) tok.error_exit("importにはファイルパスの文字列が必要です");
        std::string_view path_tok = tok.top().token;
        std::string path(path_tok.begin() + 1, path_tok.end() - 1);
        tok.next_token();
        if(tok.top().token != "as") tok.error_exit("importには 'as' が必要です");
        tok.next_token();
        if(tok.top().type != token_t::IDENT) tok.error_exit("importのasの後には識別子が必要です");
        std::string alias(tok.top().token);
        tok.next_token();

        std::ifstream ifile(path);
        if(!ifile) tok.error_exit("インポート対象のファイルを開けませんでした: " + path);
        std::string import_src((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
        
        tokenize import_tok(import_src);
        AST::Nstat* import_ast = new AST::Nstat();
        try {
          second(import_tok, false, import_ast, alias + "::");
        } catch (const except& e) {
          delete import_ast;
          if(e == except::EMPTY) {
          } else throw;
        }
        for(auto item : import_ast->items){
          parent->items.push_back(item);
        }
        import_ast->items.clear();
        delete import_ast;
      }else parent->items.push_back(EXPR::expr(tok));
    }catch(const except&e){
      if(e==except::EMPTY) break;
      else throw e;
    }
    skip_sep(tok);
    if(tok.top().symbol==symbol_t::RCURLY) break;
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
AST::Ndecl* define_var(tokenize&tok,AST::Nstat*parent,std::string_view ns_prefix){
  tok.next_token();
  if(tok.top().type!=token_t::IDENT)
    tok.error_exit(__func__+std::string(" : 変数名に使えない文字列が含まれてるかもね"));
  std::string_view variable=tok.top().token;
  if(auto err=check_ident(variable);err!=ident_error::OK)
    tok.error_exit(__func__+std::string(" : ")+error_ident(err,false));
  
  std::string full_var_name = std::string(ns_prefix) + std::string(variable);
  tok.next_token();
  if(tok.top().type!=token_t::SYMBOL||(tok.top().symbol!=symbol_t::ASSIGN&&tok.top().symbol!=symbol_t::EQ))
    tok.error_exit(__func__+std::string(" : 宣言文では=か:=で代入してください．"));
  tok.next_token();
  AST::Nitem* expr=EXPR::expr(tok); // 例外は上へ
  if(parent&&parent->var_names_set.emplace(full_var_name).second) // set追加に失敗したら戻り値はfalse
    parent->var_names.emplace_back(full_var_name);
  return new AST::Ndecl(full_var_name,expr,parent);
}

void define_fn(tokenize&tok,std::string_view ns_prefix){
  tok.next_token();
  if(tok.top().type!=token_t::IDENT)
    tok.error_exit(__func__+std::string(" : 関数名に使えない文字列が含まれてるかもね"));
  std::string_view fn_name=tok.top().token;
  if(auto err=check_ident(fn_name);err!=ident_error::OK)
    tok.error_exit(__func__+std::string(" : ")+error_ident(err,true));
  std::string full_fn_name = std::string(ns_prefix) + std::string(fn_name);
  std::unordered_set<std::string,MAP_VAR_FN::StringHash,MAP_VAR_FN::StringEqual> Sargs;
  std::vector<std::string> args;
  AST::Nstat*body=new AST::Nstat();
  tok.next_token();
  if(tok.top().symbol!=symbol_t::LPAREN)
    tok.error_exit(__func__+std::string(" : 関数の引数は()で囲んでください"));
  tok.next_token();
  while(tok.top().type==token_t::IDENT){ // 引数
    if(auto err=check_ident(tok.top().token);err!=ident_error::OK)
      tok.error_exit(__func__+std::string(" : ")+error_ident(err,false));
    if(!Sargs.emplace(tok.top().token).second)
      tok.error_exit(__func__+std::string(" : 関数")+std::string(fn_name)+"の引数名が重複しています");
    args.emplace_back(tok.top().token);
    tok.next_token();
    if(tok.top().symbol==symbol_t::COMMA) tok.next_token();
    else break;
  }
  if(tok.top().symbol!=symbol_t::RPAREN)
    tok.error_exit(__func__+std::string(" : 関数の引数は()で囲んでください"));
  tok.next_token();
  if(tok.top().symbol!=symbol_t::LCURLY)
    tok.error_exit(__func__+std::string(" : 関数の定義は{}で囲んでください"));
  tok.next_token(); // {を消費
  // 先に関数名だけ登録
  auto [it,success]=AST::fn_map.emplace(full_fn_name,nullptr);
  if(!success)
    tok.error_exit(__func__+std::string(" : 関数")+full_fn_name+"が重複しています");
  second(tok,true,body);
  if(tok.top().symbol!=symbol_t::RCURLY) tok.error_exit(__func__+std::string(" : 関数")+full_fn_name+"の定義が閉じられていません");
  tok.next_token(); // }を消費
  body->args=std::move(args);
  body->args_set=std::move(Sargs);
  it->second=body; // 関数の中身を登録
}
} // namespace PARSER