#include "reserved.hpp"
#include <iostream>

namespace AST{
expr_t print(std::vector<Nitem*>&args){
  for(auto&&expr:args){
    expr_t arg=expr->get_value();
    if(std::holds_alternative<bint>(arg))
      std::cout<<std::get<bint>(arg)<<std::endl;
    else if(std::holds_alternative<bfloat>(arg))
      std::cout<<std::get<bfloat>(arg)<<std::endl;
    else if(std::holds_alternative<bool>(arg))
      std::cout<<std::boolalpha<<std::get<bool>(arg)<<std::endl;
  }
  return bint(args.size());
}
}//namespace EXPR}