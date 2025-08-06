#include "reserved.hpp"
namespace AST{

expr_t abs(std::vector<Nitem*>&args){
  if(args.size()!=1) throw std::runtime_error("absの引数が1つではありません");
  expr_t arg=args[0]->get_value();
  if(std::holds_alternative<bool>(arg)) return arg;
  if(std::holds_alternative<bint>(arg)){
    if(std::get<bint>(arg)<0) arg=-std::get<bint>(arg);
    return arg;
  }
  return mp::abs(std::get<bfloat>(arg));
}

expr_t log(std::vector<Nitem*>&args){
  if(args.size()!=2) throw std::runtime_error("logの引数が異なります．");
  expr_t arg=args[1]->get_value();
  bool is_arg_int=std::holds_alternative<bint>(arg);
  expr_t value=mp::log(is_arg_int?(bfloat)std::get<bint>(arg):std::get<bfloat>(arg));
  if(args[0]!=nullptr){
    bool is_base_int=std::holds_alternative<bint>(args[0]->get_value());
    expr_t div=mp::log(is_base_int?(bfloat)std::get<bint>(args[0]->get_value()):std::get<bfloat>(args[0]->get_value()));
    std::get<bfloat>(value)/=std::get<bfloat>(div);
  }
  return value;
}

template<auto pred>
expr_t powerable(std::vector<Nitem*>&args){
  if(args.size()!=1) throw std::runtime_error("引数が1つではありません");
  expr_t arg=args[0]->get_value();
  if(std::holds_alternative<bool>(arg)) arg=(bfloat)(int)std::get<bool>(arg);
  else if(std::holds_alternative<bint>(arg)) arg=(bfloat)std::get<bint>(arg);
  return pred(std::move(std::get<bfloat>(arg)));
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
#undef defp

}//namespace AST