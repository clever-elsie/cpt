#include "tokenize.hpp"
#include "expr/expr.hpp"
#include "top.hpp"
#include <string>
#include <cstddef>
#include <cctype>

namespace CALC{

bool define_fn(tokenize&tok);
expr_t define_var(tokenize&tok);

bool top(std::string_view istr){
  static expr_t hold;
  tokenize tok(istr);
  while(true){
    try{
      if(tok.top().token=="let") hold=define_var(tok);
      else if(tok.top().token=="def"){
        if(!define_fn(tok)){
          std::cerr<<"関数の定義は未実装です"<<std::endl;
          exit(EXIT_FAILURE);
        }
      }else hold=EXPR::expr(tok);
    }catch(const except&e){
      if(e==except::EMPTY) break;
      else return false;
    }
    if(tok.top().token==";"||tok.top().token==",") tok.next_token();
  }
  std::cout<<std::boolalpha;
  std::visit([](const auto&x){
    std::cout<<x<<std::endl;
  },hold);
  return true;
}

expr_t define_var(tokenize&tok){
  tok.next_token();
  if(tok.top().type!=token_t::IDENT){
    std::cerr<<"変数名に使えない文字列が含まれてるかもね"<<std::endl;
    return false;
  }
  std::string_view variable=tok.top().token;
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
  // 実装は任せた
  return false;
}
}// namespace CALC