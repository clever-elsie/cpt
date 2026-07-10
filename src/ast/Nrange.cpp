#include "ast/ast.hpp"

namespace AST {

Nrange::Nrange(size_t row, size_t col, Nitem* start_expr, Nitem* end_expr, bool is_inclusive)
  : Nitem(row, col), start_expr(start_expr), end_expr(end_expr), is_inclusive(is_inclusive) {}

Nrange::~Nrange() {
  delete start_expr;
  delete end_expr;
}

expr_t Nrange::get_value() {
  expr_t sv = start_expr->get_value();
  expr_t ev = end_expr->get_value();
  if(!sv.is<expr_t::types::BINT>() || !ev.is<expr_t::types::BINT>())
    throw std::runtime_error("範囲演算の開始および終了値は整数でなければなりません");
  auto r = std::make_shared<Range>();
  r->start = sv.get<bint>();
  r->end = ev.get<bint>();
  r->is_inclusive = is_inclusive;
  return expr_t(r);
}

} // namespace AST
