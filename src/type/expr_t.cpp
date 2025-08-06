#include "expr_t.hpp"
#include <cstdint>


expr_t operator+=(expr_t&lhs,expr_t&&rhs){ return lhs+=rhs; }
expr_t operator-=(expr_t&lhs,expr_t&&rhs){ return lhs-=rhs; }
expr_t operator*=(expr_t&lhs,expr_t&&rhs){ return lhs*=rhs; }
expr_t operator/=(expr_t&lhs,expr_t&&rhs){ return lhs/=rhs; }
expr_t operator%=(expr_t&lhs,expr_t&&rhs){ return lhs%=rhs; }

expr_t operator+=(expr_t&lhs,expr_t&rhs){
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

expr_t operator-=(expr_t&lhs,expr_t&rhs){
  bool p=std::holds_alternative<bint>(lhs);
  if(p==std::holds_alternative<bint>(rhs))
    if(p) std::get<bint>(lhs)-=std::get<bint>(rhs);
    else std::get<bfloat>(lhs)-=std::get<bfloat>(rhs);
  else
    if(p) lhs=static_cast<bfloat>(get<bint>(lhs))-get<bfloat>(rhs);
    else get<bfloat>(lhs)-=static_cast<bfloat>(get<bint>(rhs));
  return lhs;
}

expr_t operator*=(expr_t&lhs,expr_t&rhs){
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

expr_t operator/=(expr_t&lhs,expr_t&rhs){
  bool zerodiv=false;
  if(std::holds_alternative<bint>(rhs))
    zerodiv=get<bint>(rhs)==0;
  else zerodiv=get<bfloat>(rhs)==0;
  if(zerodiv) throw std::runtime_error("ゼロ除算は禁止");
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

expr_t operator%=(expr_t&lhs,expr_t&rhs){
  if(std::holds_alternative<bint>(lhs)&&std::holds_alternative<bint>(rhs)){
    if(std::get<bint>(rhs)==0)
      throw std::runtime_error("ゼロ除算は禁止");
    std::get<bint>(lhs)%=std::get<bint>(rhs);
  }else throw std::runtime_error("整数以外の型では剰余演算はできません");
  return lhs;
}

/**
 * @brief 正の整数の指数のべき乗を計算する
 */
expr_t pow(bint&lhs, bint&rhs){
  bint ret=1;
  do{
    uint64_t r=rhs.convert_to<uint64_t>();
    rhs>>=64;
    if(rhs>0) // 続きあり
      for(char i=0;i<64;++i,r>>=1){
        if(r&1) ret*=lhs;
        lhs*=lhs;
      }
    else{ // このループで終了
      for(;r;r>>=1){
        if(r&1) ret*=lhs;
        lhs*=lhs;
      }
      break;
    }
  }while(1);
  return ret;
}

expr_t pow(expr_t&lhs, expr_t&rhs){
  bool l=holds_alternative<bint>(lhs);
  bool r=holds_alternative<bint>(rhs);
  if(l&&r&&std::get<bint>(rhs)>0)
    return pow(get<bint>(lhs),get<bint>(rhs));
  return mp::pow(l?(bfloat)get<bint>(lhs):get<bfloat>(lhs),r?(bfloat)get<bint>(rhs):get<bfloat>(rhs));
}

template<class Pred1,class Pred2,class Pred3>
bool compare(const expr_t&lhs,const expr_t&rhs){
  constexpr static Pred1 pred1{};
  constexpr static Pred2 pred2{};
  constexpr static Pred3 pred3{};
  bool l=std::holds_alternative<bfloat>(lhs);
  bool r=std::holds_alternative<bfloat>(rhs);
  if(l||r){ // bfloat
    if(l&&r) return pred1(std::get<bfloat>(lhs),std::get<bfloat>(rhs)); // bfloat==bfloat
    auto&a=l?std::get<bfloat>(lhs):std::get<bfloat>(rhs); // bfloatの方
    auto&b=l? // bintかboolの方をbintで参照
      std::holds_alternative<bint>(rhs)?
        std::get<bint>(rhs)
       :(int)std::get<bool>(rhs)
     :std::holds_alternative<bint>(lhs)?
        std::get<bint>(lhs)
       :(int)std::get<bool>(lhs);
    return pred1(a,static_cast<bfloat>(b));
  }
  l=std::holds_alternative<bint>(lhs);
  r=std::holds_alternative<bint>(rhs);
  if(l||r){ // bint
    if(l&&r) return pred2(std::get<bint>(lhs),std::get<bint>(rhs)); // bint==bint
    if(l) return pred2(std::get<bint>(lhs),(int)std::get<bool>(rhs)); // bint==bool
    return pred2((int)std::get<bool>(lhs),std::get<bint>(rhs)); // bool==bint
  }
  return pred3(std::get<bool>(lhs),std::get<bool>(rhs)); // bool==bool
}

bool operator==(const expr_t&lhs,const expr_t&rhs){
  return compare<std::equal_to<bfloat>,std::equal_to<bint>,std::equal_to<bool>>(lhs,rhs);
}

bool operator<(const expr_t&lhs,const expr_t&rhs){
  return compare<std::less<bfloat>,std::less<bint>,std::less<bool>>(lhs,rhs);
}

bool to_bool(const expr_t&value){
  if(std::holds_alternative<bint>(value))
    return std::get<bint>(value)!=0;
  else if(std::holds_alternative<bfloat>(value))
    return std::get<bfloat>(value)!=0;
  else if(std::holds_alternative<bool>(value))
    return std::get<bool>(value);
  throw std::runtime_error("無効な型");
  return false;
}

bool operator&&(const expr_t&lhs,const expr_t&rhs){
  return to_bool(lhs)&&to_bool(rhs);
}

bool operator||(const expr_t&lhs,const expr_t&rhs){
  return to_bool(lhs)||to_bool(rhs);
}