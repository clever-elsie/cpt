#include "reserved.hpp"
#include "misc.hpp"
namespace CALC{namespace EXPR{

expr_t abs(tokenize&tok){
  expr_t arg=get_arg(tok);
  if(std::holds_alternative<bint>(arg)){
    if(std::get<bint>(arg)<0) arg=-std::get<bint>(arg);
    return arg;
  }
  return mp::abs(std::get<bfloat>(arg));
}

expr_t log(tokenize&tok){
  std::optional<expr_t> e,b;
  for(int i=0;i<2;++i){
    if(tok.top().token=="_"){
      if(b.has_value()) tok.error_exit(__func__+std::string(" : 下付きが多すぎます"));
      b=get_idx(tok);
    }else if(tok.top().token=="^"){
      if(e.has_value()) tok.error_exit(__func__+std::string(" : 上付きが多すぎます"));
      e=get_idx(tok);
    }else break;
  }
  expr_t arg=get_arg(tok);
  expr_t value=mp::log(
    std::holds_alternative<bint>(arg)?
      static_cast<bfloat>(std::get<bint>(arg))
      :std::get<bfloat>(arg));
  if(b.has_value()){
    expr_t div=mp::log(std::holds_alternative<bint>(b.value())?
      static_cast<bfloat>(std::get<bint>(b.value()))
      :std::get<bfloat>(b.value()));
    std::get<bfloat>(value)/=std::get<bfloat>(div);
  }
  if(e.has_value()) value=pow(value,e.value());
  return value;
}

template<auto pred>
expr_t powerable(tokenize&tok){
  std::optional<expr_t> e{};
  if(tok.top().token=="^") e=get_idx(tok);
  expr_t arg=get_arg(tok);
  expr_t value=pred(
    std::holds_alternative<bint>(arg)?
      static_cast<bfloat>(std::get<bint>(arg))
      :std::get<bfloat>(arg));
  if(e.has_value()) value=pow(value,e.value());
  return value;
}
#define defp(name) \
  expr_t name(tokenize&tok)noexcept(false){\
    return powerable<[](bfloat&&e){return mp::name(e);}>(tok);\
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

}}//namespace EXPR}CALC}