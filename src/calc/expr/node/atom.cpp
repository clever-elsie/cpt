#include "../node.hpp"
#include "reserved/reserved.hpp"
#include <array>
#include <algorithm>
#include <ranges>

namespace CALC{namespace EXPR{

expr_t atom(tokenize&tok) noexcept(false) {
  if(token_t::EMPTY==tok.top().type){
    std::cerr<<"項が空文字列です"<<std::endl;
    exit(EXIT_FAILURE);
  }else if(token_t::IDENT==tok.top().type){
    if(auto ftr=fn_map.find(tok.top().token);ftr!=fn_map.end()){ // 関数
      // fn_name(arg1...)
      std::vector<expr_t> args;
      tok.next_token(); // 関数名を消費
      for(size_t i=0;i<ftr->second.args.size();++i){ // ,区切りの式
        tok.next_token(); // (や,を消費
        expr_t arg=expr(tok);
        args.push_back(std::move(arg));
      }
      //std::cout<<"call "<<ftr->first<<" "<<get<bint>(args[0])<<std::endl;
      if(args.size()!=ftr->second.args.size()){
        std::cerr<<"関数"<<ftr->first<<"の引数数が一致しません（期待:"<<ftr->second.args.size()<<", 実際:"<<args.size()<<"）"<<std::endl;
        exit(EXIT_FAILURE);
      }
      if(tok.top().token==")") tok.next_token();
      else{
        std::cerr<<"関数呼び出しでかっこが閉じられてないよ"<<std::endl;
        exit(EXIT_FAILURE);
      }
      for(auto&&[arg,var]:std::views::zip(ftr->second.args,args)){
        if(auto itr=var_map.find(arg);itr==var_map.end())
          var_map.emplace(arg,std::vector<expr_t>{std::move(var)});
        else itr->second.push_back(std::move(var));
      }
      std::string_view bodyv=ftr->second.body;
      CALC::tokenize body(bodyv);
      expr_t ret=CALC::second(body,true);
      for(auto&&arg:ftr->second.args){
        if(auto itr=var_map.find(arg);itr!=var_map.end()&&itr->second.size()>0)
          itr->second.pop_back();
        else {
          std::cerr<<"関数"<<ftr->first<<"の引数"<<arg<<"の破棄に失敗しました"<<std::endl;
          exit(EXIT_FAILURE);
        }
      }
      for(auto&&var:ftr->second.vars){
        if(auto itr=var_map.find(var);itr!=var_map.end()&&itr->second.size()>0)
          itr->second.pop_back();
        else {
          std::cerr<<"関数"<<ftr->first<<"の変数"<<var<<"の破棄に失敗しました"<<std::endl;
          exit(EXIT_FAILURE);
        }
      }
      return ret;
    }else{// 変数
      auto itr=var_map.find(tok.top().token);
      if(itr==var_map.end()||itr->second.size()==0){
        std::cerr<<"未定義変数"<<tok.top().token<<"を使ってるよ"<<std::endl;
        exit(EXIT_FAILURE);
      }
      tok.next_token();
      return itr->second.back();
    }
  }else if(token_t::RESERVED==tok.top().type)
    return reserved_functions(tok);
  else if(tok.top().token=="!"){
    tok.next_token();
    expr_t value=atom(tok);
    if(std::holds_alternative<bool>(value)){
      return expr_t(!std::get<bool>(value));
    }else{
      std::cerr<<"無効な論理否定"<<std::endl;
      exit(EXIT_FAILURE);
    }
  }else if(tok.top().token=="-"){
    tok.next_token();
    expr_t value=atom(tok);
    if(std::holds_alternative<bool>(value)){
      value=bint(-static_cast<int>(std::get<bool>(value)));
    }else if(std::holds_alternative<bint>(value)){
      value=-std::get<bint>(value);
    }else if(std::holds_alternative<bfloat>(value)){
      value=-std::get<bfloat>(value);
    }else{
      std::cerr<<"無効な負の符号演算"<<std::endl;
      exit(EXIT_FAILURE);
    }
    return value;
  }else if(tok.top().token=="("){
    tok.next_token();
    expr_t value=expr(tok);
    if(tok.top().token==")") tok.next_token();
    else{
      std::cerr<<"かっこが閉じられてないよ"<<std::endl;
      exit(EXIT_FAILURE);
    }
    return value;
  }else if(token_t::DECIMAL==tok.top().type) return parse_DECIMAL(tok);
  else if(token_t::FLOAT==tok.top().type) return parse_FLOAT(tok);
  else if(token_t::BINARY==tok.top().type) return parse_BINARY(tok);
  else if(token_t::HEX==tok.top().type) return parse_HEX(tok);
  else if(token_t::SYMBOL==tok.top().type){
    std::cerr<<tok.top().token<<"は無効な記号です"<<std::endl;
    exit(EXIT_FAILURE);
  }
  return expr_t(false);
}

using R_R=expr_t(*)(tokenize&)noexcept(false);
using psr=std::pair<std::string_view,R_R>;
constexpr auto construct_fn_list()noexcept(true){
#define r(name) {#name,&name}
  std::array<psr,15> fnlist{
    (psr)r(abs),
    r(sum), r(prod),
    r(log), r(log10),
    r(cos), r(sin), r(tan),
    r(acos), r(asin), r(atan),
    r(cosh), r(sinh), r(tanh),
    r(print)
  };
#undef r
  std::sort(fnlist.begin(),fnlist.end(),[](const psr&l,const psr&r){
    return l.first<r.first;
  });
  return fnlist;
}

expr_t reserved_functions(tokenize&tok)noexcept(false){
  std::string_view id=tok.top().token.substr(1);
  tok.next_token();
  const static constexpr auto fnlist=construct_fn_list();
  auto itr=std::lower_bound(fnlist.begin(),fnlist.end(),id,[](const psr&l,const std::string_view&r){
    return l.first<r;
  });
  if(itr==fnlist.end()||itr->first!=id){
    std::cerr<<id<<"は未定義関数です"<<std::endl;
    exit(EXIT_FAILURE);
  }
  return itr->second(tok);
}

}} // namespace EXPR}CALC}