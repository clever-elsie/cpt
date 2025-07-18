#include "type.hpp"
namespace CALC{
expr_t operator+=(expr_t&lhs,expr_t&&rhs){
  bool p=std::holds_alternative<bint>(lhs);
  if(p==std::holds_alternative<bint>(rhs))
    if(p) std::get<bint>(lhs)+=std::get<bint>(rhs);
    else std::get<bfloat>(lhs)+=std::get<bfloat>(rhs);
  else{
    if(p) swap(lhs,rhs);
    std::get<bfloat>(lhs)+=static_cast<bfloat>(std::get<bint>(rhs));
  }
  return lhs;
}

expr_t operator-=(expr_t&lhs,expr_t&&rhs){
  bool p=std::holds_alternative<bint>(lhs);
  if(p==std::holds_alternative<bint>(rhs))
    if(p) std::get<bint>(lhs)-=std::get<bint>(rhs);
    else std::get<bfloat>(lhs)-=std::get<bfloat>(rhs);
  else
    if(p) lhs=static_cast<bfloat>(get<bint>(lhs))-get<bfloat>(rhs);
    else get<bfloat>(lhs)-=static_cast<bfloat>(get<bint>(rhs));
  return lhs;
}

expr_t operator*=(expr_t&lhs,expr_t&&rhs){
  bool p=std::holds_alternative<bint>(lhs);
  if(p==std::holds_alternative<bint>(rhs))
    if(p) std::get<bint>(lhs)*=std::get<bint>(rhs);
    else std::get<bfloat>(lhs)*=std::get<bfloat>(rhs);
  else{
    if(p) std::swap(lhs,rhs);
    std::get<bfloat>(lhs)*=static_cast<bfloat>(std::get<bint>(rhs));
  }
  return lhs;
}

expr_t operator/=(expr_t&lhs,expr_t&&rhs){
  bool zerodiv=false;
  if(std::holds_alternative<bint>(rhs))
    zerodiv=get<bint>(rhs)==0;
  else zerodiv=get<bfloat>(rhs)==0;
  if(zerodiv){
    std::cerr<<"ゼロ除算は禁止"<<std::endl;
    exit(EXIT_FAILURE);
  }
  bool p=std::holds_alternative<bint>(lhs);
  if(p==std::holds_alternative<bint>(rhs))
    if(p){
      bint q,r;
      divide_qr(get<bint>(lhs),get<bint>(rhs),q,r); // ADL
      if(r==0) get<bint>(lhs)=q;
      else lhs=expr_t(static_cast<bfloat>(get<bint>(lhs))/static_cast<bfloat>(get<bint>(rhs)));
    }else std::get<bfloat>(lhs)/=std::get<bfloat>(rhs);
  else{
    if(p) lhs=expr_t(static_cast<bfloat>(get<bint>(lhs))/get<bfloat>(rhs));
    else get<bfloat>(lhs)/=static_cast<bfloat>(get<bint>(rhs));
  }
  return lhs;
}

expr_t pow(const expr_t&lhs, const expr_t& rhs){
  bool l=holds_alternative<bint>(lhs);
  bool r=holds_alternative<bint>(rhs);
  return mp::pow(l?static_cast<bfloat>(get<bint>(lhs)):get<bfloat>(lhs),
    r?static_cast<bfloat>(get<bint>(rhs)):get<bfloat>(rhs));
}

bool operator==(expr_t&lhs,expr_t&rhs){
  if(std::holds_alternative<bool>(lhs)) lhs=expr_t(bint((int)get<bool>(lhs)));  
  if(std::holds_alternative<bool>(rhs)) rhs=expr_t(bint((int)get<bool>(rhs)));  
  bool l=std::holds_alternative<bint>(lhs);
  bool r=std::holds_alternative<bint>(rhs);
  if(l&&r) return get<bint>(lhs)==get<bint>(rhs);
  if(l) return static_cast<bfloat>(get<bint>(lhs))==get<bfloat>(rhs);
  if(r) return get<bfloat>(lhs)==static_cast<bfloat>(get<bint>(rhs));
  return get<bfloat>(lhs)==get<bfloat>(rhs);
}

bool operator<(expr_t&lhs,expr_t rhs){
  if(std::holds_alternative<bool>(lhs)) lhs=expr_t(bint((int)get<bool>(lhs)));  
  if(std::holds_alternative<bool>(rhs)) rhs=expr_t(bint((int)get<bool>(rhs)));  
  bool l=std::holds_alternative<bint>(lhs);
  bool r=std::holds_alternative<bint>(rhs);
  if(l&&r) return get<bint>(lhs)<get<bint>(rhs);
  if(l) return static_cast<bfloat>(get<bint>(lhs))<get<bfloat>(rhs);
  if(r) return get<bfloat>(lhs)<static_cast<bfloat>(get<bint>(rhs));
  return get<bfloat>(lhs)<get<bfloat>(rhs);
}
}//namespace CALC