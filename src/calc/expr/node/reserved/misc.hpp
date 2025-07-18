#include "reserved.hpp"
#include <optional>
namespace CALC{namespace EXPR{

inline expr_t get_arg(tokenize&tok)noexcept(false){
  expr_t arg;
  if(tok.top().type==token_t::SYMBOL&&tok.top().token=="("){
    tok.next_token();
    arg=expr(tok);
    if(tok.top().type!=token_t::SYMBOL||tok.top().token!=")"){
      std::cerr<<"かっこが閉じられていません"<<std::endl;
      exit(EXIT_FAILURE);
    }
    tok.next_token();
  }else arg=term(tok);
  if(std::holds_alternative<bool>(arg))
    arg=bint((int)std::get<bool>(arg));
  return arg;
}

// first.token=='_'の状態で使う
inline std::pair<std::string,expr_t> get_below_with_declare(tokenize&tok)noexcept(false){
  tok.next_token();
  if(tok.top().token!="{"){
    std::cerr<<"下付き引数には{}が必要です"<<std::endl;
    exit(EXIT_FAILURE);
  }
  tok.next_token();
  if(tok.top().type!=token_t::IDENT){
    std::cerr<<"変数が必要です"<<std::endl;
    exit(EXIT_FAILURE);
  }
  const std::string name(tok.top().token);
  tok.next_token();
  if(tok.top().token!="="){
    std::cerr<<"変数は初期化してください"<<std::endl;
    exit(EXIT_FAILURE);
  }    
  expr_t init=expr(tok);
  if(!std::holds_alternative<bint>(init)){
    std::cerr<<"ループ変数の初期値は整数である必要があります"<<std::endl;
    exit(EXIT_FAILURE);
  }
  if(tok.top().token!="}"){
    std::cerr<<"下付き引数の{}が閉じられていません"<<std::endl;
    exit(EXIT_FAILURE);
  }
  tok.next_token();
  var_map[name].push_back(init);
  return {std::move(name),std::move(init)};
}

inline expr_t get_idx(tokenize&tok)noexcept(false){
  tok.next_token();
  if(tok.top().token=="{"){
    tok.next_token();
    expr_t init=expr(tok);
    if(tok.top().token!="}"){
      std::cerr<<"下付き引数の{}が閉じられていません"<<std::endl;
      exit(EXIT_FAILURE);
    }
    tok.next_token();
    return init;
  }
  //{}が無いときはatom
  return atom(tok);
  // リテラルとか変数とかが一つだけなら{}いらないと思ったんですが，これだと色々できますよね……
}

}}//namespace EXPR}CALC}