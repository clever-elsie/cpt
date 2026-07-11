#include "ast/reserved.hpp"
#include <iostream>

namespace AST{

static void print_expr_internal(const expr_t& e) {
  expr_t val = e.deref();
  if (val.is<expr_t::types::BINT>()) {
    std::cout << val.get<bint>();
  } else if (val.is<expr_t::types::BFLOAT>()) {
    std::cout << val.get<bfloat>();
  } else if (val.is<expr_t::types::BOOL>()) {
    std::cout << std::boolalpha << val.get<bool>();
  } else if (val.is<expr_t::types::COMPLEX>()) {
    auto c = val.get<bcomplex>();
    std::cout << c.real();
    if (c.imag() >= 0) std::cout << "+" << c.imag() << "i";
    else std::cout << c.imag() << "i";
  } else if (val.is<expr_t::types::STRING>()) {
    std::cout << val.get<std::string>();
  } else if (val.is<expr_t::types::MATRIX>()) {
    auto m = val.get<std::shared_ptr<Matrix>>();
    std::cout << "[";
    for (size_t i=0; i<m->rows; ++i) {
      for (size_t j=0; j<m->cols; ++j) {
        print_expr_internal(m->data[i * m->cols + j]);
        if (j + 1 < m->cols) std::cout << ", ";
      }
      if (i + 1 < m->rows) std::cout << "; ";
    }
    std::cout << "]";
  } else if (val.is<expr_t::types::RANGE>()) {
    auto r = val.get<std::shared_ptr<Range>>();
    std::cout << r->start << (r->is_inclusive ? "..=" : "..") << r->end;
  }
}

expr_t print(std::vector<Nitem*>&args){
  for(auto&&expr:args){
    expr_t arg=expr->get_value();
    print_expr_internal(arg);
    std::cout<<std::endl;
  }
  return bint(args.size());
}
}//namespace EXPR