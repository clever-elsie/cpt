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

} // namespace AST
