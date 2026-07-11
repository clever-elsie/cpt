#include "ast/ast.hpp"

namespace AST {

Nfor::Nfor(size_t row, size_t col, std::string_view var_name, Nitem* range_expr, Nitem* body)
  : Nitem(row, col), var_name(var_name), range_expr(range_expr), body(body) {}

Nfor::~Nfor() {
  delete range_expr;
  delete body;
}

expr_t Nfor::get_value() {
  expr_t range_val = range_expr->get_value().deref();
  std::vector<expr_t> elements;
  if(range_val.is<expr_t::types::RANGE>()) {
    auto r = range_val.get<std::shared_ptr<Range>>();
    bint start = r->start;
    bint end = r->end;
    bool inclusive = r->is_inclusive;
    bint current = start;
    while(true){
      bool cond = inclusive ? (current <= end) : (current < end);
      if(!cond) break;
      elements.push_back(expr_t(current));
      current += 1;
    }
  } else if(range_val.is<expr_t::types::MATRIX>()) {
    elements = range_val.get<std::shared_ptr<Matrix>>()->get_iterable_elements();
  } else {
    throw std::runtime_error("forループの対象がイテレート可能ではありません");
  }

  expr_t last_val = expr_t(std::monostate{});
  auto& vec = var_map[var_name];

  for(auto& val : elements){
    vec.emplace_back(val);
    try {
      last_val = body->get_value();
    } catch (...) {
      vec.pop_back();
      throw;
    }
    vec.pop_back();
  }

  return last_val;
}

json::value Nfor::to_json() const {
  json::value v;
  v["type"] = "for";
  v["var_name"] = var_name;
  v["range"] = range_expr ? range_expr->to_json() : json::value();
  v["body"] = body ? body->to_json() : json::value();
  v["row"] = (int64_t)row;
  v["col"] = (int64_t)col;
  return v;
}

} // namespace AST
