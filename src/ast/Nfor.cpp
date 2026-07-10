#include "ast/ast.hpp"

namespace AST {

Nfor::Nfor(size_t row, size_t col, std::string_view var_name, Nitem* range_expr, Nitem* body)
  : Nitem(row, col), var_name(var_name), range_expr(range_expr), body(body) {}

Nfor::~Nfor() {
  delete range_expr;
  delete body;
}

expr_t Nfor::get_value() {
  expr_t range_val = range_expr->get_value();
  if(!range_val.is<expr_t::types::RANGE>())
    throw std::runtime_error("forループの対象がRangeオブジェクトではありません");
  
  auto r = range_val.get<std::shared_ptr<Range>>();
  bint start = r->start;
  bint end = r->end;
  bool inclusive = r->is_inclusive;

  expr_t last_val = expr_t(std::monostate{});

  auto& vec = var_map[var_name];

  bint current = start;
  while(true){
    bool cond = inclusive ? (current <= end) : (current < end);
    if(!cond) break;

    vec.emplace_back(current);
    try {
      last_val = body->get_value();
    } catch (...) {
      vec.pop_back();
      throw;
    }
    vec.pop_back();
    current += 1;
  }

  return last_val;
}

} // namespace AST
