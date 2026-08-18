#pragma once
#include <string>
namespace INPUT{
  enum class input_mode{
    standard_input,
    command_line,
    file,
  };

  extern int print_precision;
  extern bool dump_ast;
  extern input_mode mode;
  extern std::string source_name;
  std::string get_all_source_input(int argc, char**argv);
}
