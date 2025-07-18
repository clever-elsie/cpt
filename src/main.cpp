#include <iostream>
#include "input.hpp"
#include "calc/top.hpp"


int main(int argc, char**argv){
  std::string src = INPUT::get_all_source_input(argc,argv);
  if(!CALC::top(std::string_view(src.begin(),src.end())))
    std::cerr<<"ERROR"<<std::endl;
}