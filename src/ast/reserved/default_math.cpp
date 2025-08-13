#include "ast/reserved.hpp"
namespace AST{

expr_t abs(std::vector<Nitem*>&args){
  if(args.size()!=1) throw std::runtime_error("absの引数が1つではありません");
  expr_t arg=args[0]->get_value();
  if(arg.is<expr_t::types::BOOL>()) return arg;
  if(arg.is<expr_t::types::BINT>()){
    if(arg.get<bint>()<0) arg=-arg.get<bint>();
    return arg;
  }
  return mp::abs(arg.get<bfloat>());
}

expr_t log(std::vector<Nitem*>&args){
  if(args.size()!=2) throw std::runtime_error("logの引数が異なります．");
  expr_t arg=args[1]->get_value();
  bool is_arg_int=arg.is<expr_t::types::BINT>();
  expr_t value=mp::log(is_arg_int?(bfloat)arg.get<bint>():arg.get<bfloat>());
  if(args[0]!=nullptr){
    bool is_base_int=args[0]->get_value().is<expr_t::types::BINT>();
    expr_t div=mp::log(is_base_int?(bfloat)args[0]->get_value().get<bint>():args[0]->get_value().get<bfloat>());
    value.get<bfloat>()/=div.get<bfloat>();
  }
  return value;
}

template<auto pred>
expr_t powerable(std::vector<Nitem*>&args){
  if(args.size()!=1) throw std::runtime_error("引数が1つではありません");
  expr_t arg=args[0]->get_value();
  if(arg.is<expr_t::types::BOOL>()) arg=(bfloat)(int)arg.get<bool>();
  else if(arg.is<expr_t::types::BINT>()) arg=(bfloat)arg.get<bint>();
  return pred(std::move(arg.get<bfloat>()));
}

#define defp(name) \
  expr_t name(std::vector<Nitem*>&args)noexcept(false){\
    return powerable<[](bfloat&&e){return mp::name(e);}>(args);\
  }

defp(log10)
defp(cos)
defp(sin)
defp(tan)
defp(acos)
defp(asin)
defp(atan)
defp(cosh)
defp(sinh)
defp(tanh)
defp(ceil)
defp(floor)
defp(round)
defp(trunc)
#undef defp

}//namespace AST