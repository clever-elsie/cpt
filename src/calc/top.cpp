#include "tokenize.hpp"
#include "expr/expr.hpp"
#include "top.hpp"
#include <string>
#include <cstddef>
#include <cctype>

namespace CALC{
inline VAR_MAP<expr_t> var_map;
bool define_var_fn(tokenize&tok){
  if(tok.top().token=="let"){
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
    if(auto itr=var_map.find(variable);itr==var_map.end())
      var_map.emplace(variable,std::vector<expr_t>{std::move(value)});
    else itr->second.push_back(std::move(value));
  }else if(tok.top().token=="def"){
    // 関数定義は実装後回し
  }
  return true;
}

bool top(std::string_view istr){
  static expr_t hold;
  tokenize tok(istr);
  while(true){
    try{
      bool statement=false;
      if(token_t::IDENT==tok.top().type)
        statement=tok.top().token=="let" || tok.top().token=="def";
      if(statement){
        if(!define_var_fn(tok))
          exit(EXIT_FAILURE);
      }else hold=EXPR::expr(tok);
    }catch(const except&e){
      if(e==except::EMPTY) break;
      else return false;
    }
  }
  std::cout<<std::boolalpha;
  std::visit([](const auto&x){
    std::cout<<x<<std::endl;
  },hold);
  return true;
}
}// namespace CALC