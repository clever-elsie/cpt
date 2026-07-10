#include "ast/ast.hpp"

namespace AST {

Nmatrix::Nmatrix(size_t row, size_t col, size_t rows, size_t cols, std::vector<Nitem*>&& elements)
  : Nitem(row, col), rows(rows), cols(cols), elements(std::move(elements)) {}

Nmatrix::~Nmatrix() {
  for(auto elem : elements) delete elem;
}

expr_t Nmatrix::get_value() {
  auto m = std::make_shared<Matrix>();
  m->rows = rows;
  m->cols = cols;
  m->data.reserve(elements.size());
  for(auto elem : elements){
    m->data.push_back(elem->get_value());
  }
  return expr_t(m);
}

json::value Nmatrix::to_json() const {
  json::value v;
  v["type"] = "matrix";
  v["rows"] = (int64_t)rows;
  v["cols"] = (int64_t)cols;
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
