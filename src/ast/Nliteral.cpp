#include "ast/ast.hpp"
#include <sstream>

namespace AST{

Nliteral::Nliteral(size_t row,size_t col,expr_t&&value)
:Nitem(row,col),value(std::move(value)){}

expr_t Nliteral::get_value(){
  return value;
}

json::value Nliteral::to_json() const {
  json::value v;
  v["type"] = "literal";
  if (value.is<expr_t::types::BINT>()) {
    std::ostringstream ss;
    ss << value.get<bint>();
    v["value"] = ss.str();
    v["val_type"] = "int";
  } else if (value.is<expr_t::types::BFLOAT>()) {
    std::ostringstream ss;
    ss << value.get<bfloat>();
    v["value"] = ss.str();
    v["val_type"] = "float";
  } else if (value.is<expr_t::types::BOOL>()) {
    v["value"] = value.get<bool>();
    v["val_type"] = "bool";
  } else if (value.is<expr_t::types::STRING>()) {
    v["value"] = value.get<std::string>();
    v["val_type"] = "string";
  } else if (value.is<expr_t::types::COMPLEX>()) {
    auto c = value.get<bcomplex>();
    std::ostringstream ss;
    ss << c.real() << (c.imag() >= 0 ? "+" : "") << c.imag() << "i";
    v["value"] = ss.str();
    v["val_type"] = "complex";
  } else {
    v["value"] = "other";
  }
  v["row"] = (int64_t)row;
  v["col"] = (int64_t)col;
  return v;
}

} // namespace AST