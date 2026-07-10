#include <iostream>
#include <iomanip>
#include <fstream>
#include "input/input.hpp"
#include "parser/top.hpp"

void print_expr_internal(const expr_t& e) {
  if (e.is<expr_t::types::BINT>()) {
    std::cout << e.get<bint>();
  } else if (e.is<expr_t::types::BFLOAT>()) {
    std::cout << e.get<bfloat>();
  } else if (e.is<expr_t::types::BOOL>()) {
    std::cout << std::boolalpha << e.get<bool>();
  } else if (e.is<expr_t::types::COMPLEX>()) {
    auto c = e.get<bcomplex>();
    std::cout << c.real();
    if (c.imag() >= 0) std::cout << "+" << c.imag() << "i";
    else std::cout << c.imag() << "i";
  } else if (e.is<expr_t::types::STRING>()) {
    std::cout << e.get<std::string>();
  } else if (e.is<expr_t::types::VECTOR>()) {
    std::cout << "[";
    const auto& v = e.get<std::vector<expr_t>>();
    for (size_t i=0; i<v.size(); ++i) {
      print_expr_internal(v[i]);
      if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]";
  } else if (e.is<expr_t::types::MATRIX>()) {
    auto m = e.get<std::shared_ptr<Matrix>>();
    std::cout << "[";
    for (size_t i=0; i<m->rows; ++i) {
      for (size_t j=0; j<m->cols; ++j) {
        print_expr_internal(m->data[i * m->cols + j]);
        if (j + 1 < m->cols) std::cout << ", ";
      }
      if (i + 1 < m->rows) std::cout << "; ";
    }
    std::cout << "]";
  }
}

void print_expr(const expr_t& e) {
  if (e.is<expr_t::types::VOID>()) return;
  print_expr_internal(e);
  std::cout << std::endl;
}

int main(int argc, char**argv){
  std::string src;
  try {
    src = INPUT::get_all_source_input(argc,argv);
  } catch (const std::runtime_error& e) {
    std::cerr<<e.what()<<std::endl;
    std::exit(EXIT_FAILURE);
  }

  std::ifstream tty;
  std::streambuf* cin_backup = nullptr;
  if(std::cin.eof()){
    std::cin.clear();
    tty.open("/dev/tty");
    if(tty.is_open()){
      cin_backup = std::cin.rdbuf(tty.rdbuf());
    }
  }

  if(INPUT::print_precision >= 0){
    std::cout << std::setprecision(INPUT::print_precision);
  }
  std::string file_name = "<stdin>";
  for(int i=1; i<argc; ++i){
    if(argv[i] == std::string_view("-f") && i+1 < argc){
      file_name = argv[i+1];
      break;
    }
  }
  try{
    AST::Nstat*stat=PARSER::top(std::string_view(src.begin(),src.end()), file_name);
    if(!stat){
      if(cin_backup) std::cin.rdbuf(cin_backup);
      return 0; // 空入力
    }
    expr_t ret=stat->evaluate({});
    print_expr(ret);
    delete stat;
  }catch(const std::runtime_error&e){
    std::cerr<<e.what()<<std::endl;
    if(cin_backup) std::cin.rdbuf(cin_backup);
    std::exit(EXIT_FAILURE);
  }catch(const except&e){}

  if(cin_backup) std::cin.rdbuf(cin_backup);
}