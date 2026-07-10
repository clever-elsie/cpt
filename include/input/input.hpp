#pragma once
#include <string>
namespace INPUT{
  extern int print_precision;
  extern bool dump_ast;
  std::string get_all_source_input(int argc, char**argv);
}