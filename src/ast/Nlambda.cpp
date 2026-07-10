#include "ast/ast.hpp"

namespace AST {

Nlambda::Nlambda(size_t row, size_t col, std::vector<std::string>&& args, Nstat* body)
  : Nitem(row, col), args(std::move(args)), body(body) {}

Nlambda::~Nlambda() {
  if(body) delete body;
}

expr_t Nlambda::get_value() {
  auto l = std::make_shared<LambdaFunc>();
  l->args = std::move(args);
  l->body = body;
  body = nullptr;

  for(const auto& [name, vec] : var_map){
    if(!vec.empty()){
      l->closure_env[name] = vec.back();
    }
  }

  return expr_t(l);
}

} // namespace AST
