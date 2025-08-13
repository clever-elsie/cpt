#include "ast/reserved.hpp"
#include <iostream>

namespace AST{
expr_t print(std::vector<Nitem*>&args){
  for(auto&&expr:args){
    expr_t arg=expr->get_value();
    if(arg.is<expr_t::types::BINT>())
      std::cout<<arg.get<bint>()<<std::endl;
    else if(arg.is<expr_t::types::BFLOAT>())
      std::cout<<arg.get<bfloat>()<<std::endl;
    else if(arg.is<expr_t::types::BOOL>())
      std::cout<<std::boolalpha<<arg.get<bool>()<<std::endl;
  }
  return bint(args.size());
}
}//namespace EXPR}