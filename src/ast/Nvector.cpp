#include "ast/ast.hpp"

namespace AST {

Nvector::Nvector(size_t row, size_t col, std::vector<Nitem*>&& elements)
  : Nitem(row, col), elements(std::move(elements)) {}

Nvector::~Nvector() {
  for(auto elem : elements) delete elem;
}

expr_t Nvector::get_value() {
  std::vector<expr_t> ret;
  for(auto elem : elements){
    ret.push_back(elem->get_value());
  }
  return expr_t(ret);
}

json::value Nvector::to_json() const {
  json::value v;
  v["type"] = "vector";
  json::value::array_t arr;
  for (auto elem : elements) {
    arr.push_back(elem ? elem->to_json() : json::value());
  }
  v["elements"] = arr;
  v["row"] = (int64_t)row;
  v["col"] = (int64_t)col;
  return v;
}

} // namespace AST
