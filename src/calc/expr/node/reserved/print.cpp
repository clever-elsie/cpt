#include "reserved.hpp"
#include <iostream>

namespace CALC{namespace EXPR{
expr_t print(tokenize&tok){
  if(tok.top().token!="(") tok.error_exit(__func__+std::string(" : print関数の引数はかっこで囲んでください"));
  tok.next_token();
  expr_t arg=expr(tok);
  if(tok.top().token!=")") tok.error_exit(__func__+std::string(" : print関数の引数はかっこで囲んでください"));
  tok.next_token();
  if(std::holds_alternative<bint>(arg))
    std::cout<<std::get<bint>(arg)<<std::endl;
  else if(std::holds_alternative<bfloat>(arg))
    std::cout<<std::get<bfloat>(arg)<<std::endl;
  else if(std::holds_alternative<bool>(arg))
    std::cout<<std::boolalpha<<std::get<bool>(arg)<<std::endl;
  return arg;
}
}}//namespace EXPR}CALC}