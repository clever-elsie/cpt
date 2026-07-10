#include "ast/ast.hpp"

namespace AST {

Nwhile::Nwhile(size_t row, size_t col, Nitem* cond, Nitem* body)
  : Nitem(row, col), cond(cond), body(body) {}

Nwhile::~Nwhile() {
  delete cond;
  delete body;
}

expr_t Nwhile::get_value() {
  expr_t last_val = expr_t(std::monostate{}); // VOID
  while((bool)(cond->get_value())){
    last_val = body->get_value();
  }
  return last_val;
}

} // namespace AST
