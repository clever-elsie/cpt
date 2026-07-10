#include "ast/ast.hpp"

namespace AST {

Npipeline::Npipeline(size_t row, size_t col, Nitem* lhs, Nitem* rhs)
  : Nitem(row, col), lhs(lhs), rhs(rhs) {}

Npipeline::~Npipeline() {
  delete lhs;
  delete rhs;
}

expr_t Npipeline::get_value() {
  if(auto fn_ptr = dynamic_cast<Nfn*>(rhs)){
    fn_ptr->args.insert(fn_ptr->args.begin(), lhs);
    expr_t ret;
    try {
      ret = fn_ptr->get_value();
    } catch (...) {
      fn_ptr->args.erase(fn_ptr->args.begin());
      throw;
    }
    fn_ptr->args.erase(fn_ptr->args.begin());
    return ret;
  }

  expr_t rhs_val = rhs->get_value();
  if(rhs_val.is<expr_t::types::FUNCTION>()){
    auto l = rhs_val.get<std::shared_ptr<LambdaFunc>>();
    
    std::vector<expr_t> args_eval;
    args_eval.push_back(lhs->get_value());

    for(const auto& [vname, vval] : l->closure_env){
      var_map[vname].push_back(vval);
    }

    expr_t ret;
    try {
      ret = l->body->evaluate(std::move(args_eval));
    } catch (...) {
      for(const auto& [vname, vval] : l->closure_env){
        var_map[vname].pop_back();
      }
      throw;
    }

    for(const auto& [vname, vval] : l->closure_env){
      var_map[vname].pop_back();
    }
    return ret;
  }

  throw std::runtime_error("パイプラインの右辺はイテレータアダプタ関数または関数オブジェクトでなければなりません");
}

} // namespace AST
