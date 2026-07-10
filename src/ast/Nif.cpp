#include "ast/ast.hpp"

namespace AST {

Nif::Nif(size_t row, size_t col, Nitem* cond, Nitem* then_expr, Nitem* else_expr)
  : Nitem(row, col), cond(cond), then_expr(then_expr), else_expr(else_expr) {}

Nif::~Nif() {
  delete cond;
  delete then_expr;
  if(else_expr) delete else_expr;
}

expr_t Nif::get_value() {
  expr_t cond_val = cond->get_value();
  if((bool)cond_val){
    return then_expr->get_value();
  } else if(else_expr) {
    return else_expr->get_value();
  }
  return expr_t(std::monostate{}); // VOID
}

json::value Nif::to_json() const {
  json::value v;
  v["type"] = "if";
  v["cond"] = cond ? cond->to_json() : json::value();
  v["then"] = then_expr ? then_expr->to_json() : json::value();
  if (else_expr) v["else"] = else_expr->to_json();
  v["row"] = (int64_t)row;
  v["col"] = (int64_t)col;
  return v;
}

} // namespace AST
