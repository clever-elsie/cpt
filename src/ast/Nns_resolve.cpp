#include "ast/ast.hpp"

namespace AST {

Nns_resolve::Nns_resolve(size_t row, size_t col, std::string_view alias, std::string_view name)
  : Nitem(row, col), alias(alias), name(name) {}

expr_t Nns_resolve::get_value() {
  std::string full_name = alias + "::" + name;
  auto itr = var_map.find(full_name);
  if(itr == var_map.end() || itr->second.empty())
    throw std::runtime_error("未定義の名前空間付き変数です: " + full_name);
  return itr->second.back();
}

} // namespace AST
