#include <iostream>
#include "input/input.hpp"
#include "parser/top.hpp"

int main(int argc, char**argv){
  std::string src = INPUT::get_all_source_input(argc,argv);
  try{
    AST::Nstat*stat=PARSER::top(std::string_view(src.begin(),src.end()));
    expr_t ret=stat->evaluate({});
    if(std::holds_alternative<bint>(ret))
      std::cout<<std::get<bint>(ret)<<std::endl;
    else if(std::holds_alternative<bfloat>(ret))
      std::cout<<std::get<bfloat>(ret)<<std::endl;
    else if(std::holds_alternative<bool>(ret))
      std::cout<<std::boolalpha<<std::get<bool>(ret)<<std::endl;
    delete stat;
  }catch(const std::runtime_error&e){
    std::cerr<<e.what()<<std::endl;
    std::exit(EXIT_FAILURE);
  }catch(const except&e){}
}