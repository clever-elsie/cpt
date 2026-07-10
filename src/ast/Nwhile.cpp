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

json::value Nwhile::to_json() const {
  json::value v;
  v["type"] = "while";
  v["cond"] = cond ? cond->to_json() : json::value();
  v["body"] = body ? body->to_json() : json::value();
  v["row"] = (int64_t)row;
  v["col"] = (int64_t)col;
  return v;
}

} // namespace AST
