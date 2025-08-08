#include <iostream>
#include "input/input.hpp"
#include "parser/top.hpp"

int main(int argc, char**argv){
  std::string src = INPUT::get_all_source_input(argc,argv);
  try{
    AST::Nstat*stat=PARSER::top(std::string_view(src.begin(),src.end()));
    expr_t ret=stat->evaluate({});
    if(ret.is<expr_t::types::BINT>())
      std::cout<<ret.get<bint>()<<std::endl;
    else if(ret.is<expr_t::types::BFLOAT>())
      std::cout<<ret.get<bfloat>()<<std::endl;
    else if(ret.is<expr_t::types::BOOL>())
      std::cout<<std::boolalpha<<ret.get<bool>()<<std::endl;
    delete stat;
  }catch(const std::runtime_error&e){
    std::cerr<<e.what()<<std::endl;
    std::exit(EXIT_FAILURE);
  }catch(const except&e){}
}