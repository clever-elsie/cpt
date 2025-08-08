#include "expr_t.hpp"

expr_t::types expr_t::common_type(const expr_t&lhs,const expr_t&rhs){
  types lt=lhs.type(),rt=rhs.type();
  if(lt==rt) return lt;
  if(lt==types::VECTOR||rt==types::VECTOR) throw std::runtime_error("expr_t[]と他のexpr_tの型は比較不能");
  if(lt==types::BFLOAT||rt==types::BFLOAT) return types::BFLOAT;
  if(lt==types::BINT||rt==types::BINT) return types::BINT;
  if(lt==types::BOOL||rt==types::BOOL) return types::BOOL;
  throw std::runtime_error("無効な型比較");
}

expr_t::operator bool()const{
  switch(type()){
    case types::BINT: return std::get<bint>(value)!=0;
    case types::BFLOAT: return std::get<bfloat>(value)!=0;
    case types::BOOL: return std::get<bool>(value);
    case types::VECTOR: throw std::runtime_error("無効な型変換expr_t[]->bool");
  }
  return false;
}

expr_t::operator bint()const{
  switch(type()){
    case types::BINT: return std::get<bint>(value);
    case types::BFLOAT: return static_cast<bint>(std::get<bfloat>(value));
    case types::BOOL: return std::get<bool>(value);
    case types::VECTOR: throw std::runtime_error("無効な型変換expr_t[]->bint");
  }
  return 0;
}

expr_t::operator bfloat()const{
  switch(type()){
    case types::BINT: return static_cast<bfloat>(std::get<bint>(value));
    case types::BFLOAT: return std::get<bfloat>(value);
    case types::BOOL: return std::get<bool>(value);
    case types::VECTOR: throw std::runtime_error("無効な型変換expr_t[]->bfloat");
  }
  return 0;
}

expr_t::types expr_t::type()const{
  return static_cast<types>(value.index());
}

size_t expr_t::size()const{
  if(type()==types::VECTOR) return get<std::vector<expr_t>>().size();
  return 1;
}

namespace EXPR_T_INTERNAL{
/** @brief 正の整数の指数のべき乗を計算する */
expr_t pow(bint&lhs, bint&rhs){
  bint ret=1;
  do{
    uint64_t r=rhs.convert_to<uint64_t>();
    rhs>>=64;
    if(rhs>0){// 続きあり
      for(char i=0;i<64;++i,r>>=1,lhs*=lhs)
        if(r&1) ret*=lhs;
    }else{ // このループで終了
      for(;r;r>>=1,lhs*=lhs)
        if(r&1) ret*=lhs;
      break;
    }
  }while(1);
  return ret;
}
} // namespace EXPR_T_INTERNAL

expr_t expr_t::pow(const expr_t&rhs){
  if(is<types::BINT>()&&rhs.is<types::BINT>())
    if(bint r=rhs.get<bint>();r>=0)
      return EXPR_T_INTERNAL::pow(get<bint>(),r);
  return mp::pow((bfloat)*this,(bfloat)rhs);
}

bool operator&&(const expr_t&lhs,const expr_t&rhs){
  return (bool)lhs&&(bool)rhs;
}

bool operator||(const expr_t&lhs,const expr_t&rhs){
  return (bool)lhs||(bool)rhs;
}

bool operator==(const expr_t&lhs,const expr_t&rhs){
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT:
      return (bint)lhs==(bint)rhs;
    case expr_t::types::BFLOAT:
      return (bfloat)lhs==(bfloat)rhs;
    case expr_t::types::BOOL:
      return (bool)lhs==(bool)rhs;
    case expr_t::types::VECTOR:
      const size_t lsz=lhs.size(),rsz=rhs.size();
      if(lsz!=rsz) return false;
      const auto&lv=lhs.get<std::vector<expr_t>>(),&rv=rhs.get<std::vector<expr_t>>();
      for(size_t i=0;i<lsz;++i)
        if(lv[i]!=rv[i]) return false;
      return true;
  }
  return false; // コンパイルエラー抑制
}

bool operator!=(const expr_t&lhs,const expr_t&rhs){
  return !(lhs==rhs);
}

bool operator<(const expr_t&lhs,const expr_t&rhs){
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT: return (bint)lhs<(bint)rhs;
    case expr_t::types::BFLOAT: return (bfloat)lhs<(bfloat)rhs;
    case expr_t::types::BOOL: return (bool)lhs<(bool)rhs;
    case expr_t::types::VECTOR:
      const size_t lsz=lhs.size(),rsz=rhs.size();
      const size_t minsz=std::min(lsz,rsz);
      const auto&lv=lhs.get<std::vector<expr_t>>(),&rv=rhs.get<std::vector<expr_t>>();
      for(size_t i=0;i<minsz;++i)
        if(lv[i]<rv[i]) return true;
        else if(lv[i]>rv[i]) return false;
      return lsz<rsz;
  }
  return false;
}

bool operator<=(const expr_t&lhs,const expr_t&rhs){
  return !(lhs>rhs);
}

bool operator>(const expr_t&lhs,const expr_t&rhs){
  return rhs<lhs;
}

bool operator>=(const expr_t&lhs,const expr_t&rhs){
  return !(lhs<rhs);
}

bool expr_t::operator!()const{ return !(bool)*this; }

expr_t expr_t::operator-()const{
  switch(type()){
    case types::BINT: return -(bint)*this;
    case types::BFLOAT: return -(bfloat)*this;
    case types::BOOL: return bint(-(int)(bool)*this);
    case types::VECTOR:
      const size_t lsz=size();
      std::vector<expr_t> ret(lsz);
      const auto&v=get<std::vector<expr_t>>();
      for(size_t i=0;i<lsz;++i) ret[i]=-v[i];
      return ret;
  }
  return expr_t();
}

expr_t operator+(const expr_t&lhs,const expr_t&rhs){
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT: return (bint)lhs+(bint)rhs;
    case expr_t::types::BFLOAT: return (bfloat)lhs+(bfloat)rhs;
    case expr_t::types::BOOL: return bint((int)(bool)lhs+(int)(bool)rhs);
    case expr_t::types::VECTOR:
      const size_t lsz=lhs.size();
      if(lsz!=rhs.size()) throw std::runtime_error("expr_t[]のサイズが異なります");
      const auto&lv=lhs.get<std::vector<expr_t>>(),&rv=rhs.get<std::vector<expr_t>>();
      std::vector<expr_t> ret(lsz);
      for(size_t i=0;i<lsz;++i) ret[i]=lv[i]+rv[i];
      return ret;
  }
  return expr_t();
}

expr_t operator-(const expr_t&lhs,const expr_t&rhs){
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT: return (bint)lhs-(bint)rhs;
    case expr_t::types::BFLOAT: return (bfloat)lhs-(bfloat)rhs;
    case expr_t::types::BOOL: return bint((int)(bool)lhs-(int)(bool)rhs);
    case expr_t::types::VECTOR:
      const size_t lsz=lhs.size();
      if(lsz!=rhs.size()) throw std::runtime_error("expr_t[]のサイズが異なります");
      const auto&lv=lhs.get<std::vector<expr_t>>(),&rv=rhs.get<std::vector<expr_t>>();
      std::vector<expr_t> ret(lsz);
      for(size_t i=0;i<lsz;++i) ret[i]=lv[i]-rv[i];
      return ret;
  }
  return expr_t();
}

expr_t operator*(const expr_t&lhs,const expr_t&rhs){
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT: return (bint)lhs*(bint)rhs;
    case expr_t::types::BFLOAT: return (bfloat)lhs*(bfloat)rhs;
    case expr_t::types::BOOL: return (bool)lhs&&(bool)rhs; // (int)にキャストしなくてもbool &&とint *は同型
    case expr_t::types::VECTOR:
      const size_t lsz=lhs.size();
      if(lsz!=rhs.size()) throw std::runtime_error("expr_t[]のサイズが異なります");
      const auto&lv=lhs.get<std::vector<expr_t>>(),&rv=rhs.get<std::vector<expr_t>>();
      std::vector<expr_t> ret(lsz);
      for(size_t i=0;i<lsz;++i) ret[i]=lv[i]*rv[i];
      return ret;
  }
  return expr_t();
}

expr_t operator/(const expr_t&lhs,const expr_t&rhs){
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT:{
      bint q,r;
      mp::divide_qr((bint)lhs,(bint)rhs,q,r);
      if(r==0) return q;
      return (bfloat)q+(bfloat)r/(bfloat)rhs;
    }
    case expr_t::types::BFLOAT: return (bfloat)lhs/(bfloat)rhs;
    case expr_t::types::BOOL: return bint((int)(bool)lhs/(int)(bool)rhs);
    case expr_t::types::VECTOR:
      const size_t lsz=lhs.size();
      if(lsz!=rhs.size()) throw std::runtime_error("expr_t[]のサイズが異なります");
      const auto&lv=lhs.get<std::vector<expr_t>>(),&rv=rhs.get<std::vector<expr_t>>();
      std::vector<expr_t> ret(lsz);
      for(size_t i=0;i<lsz;++i) ret[i]=lv[i]/rv[i];
      return ret;
  }
  return expr_t();
}

expr_t operator%(const expr_t&lhs,const expr_t&rhs){
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT: return (bint)lhs%(bint)rhs;
    case expr_t::types::BFLOAT: throw std::runtime_error("浮動小数点型の剰余演算は禁止");
    case expr_t::types::BOOL: return bint((int)(bool)lhs%(int)(bool)rhs);
    case expr_t::types::VECTOR:
      const size_t lsz=lhs.size();
      if(lsz!=rhs.size()) throw std::runtime_error("expr_t[]のサイズが異なります");
      const auto&lv=lhs.get<std::vector<expr_t>>(),&rv=rhs.get<std::vector<expr_t>>();
      std::vector<expr_t> ret(lsz);
      for(size_t i=0;i<lsz;++i) ret[i]=lv[i]%rv[i];
      return ret;
  }
  return expr_t();
}

expr_t idiv(const expr_t&lhs,const expr_t&rhs){
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT: return (bint)lhs/(bint)rhs;
    case expr_t::types::BFLOAT: return mp::trunc((bfloat)lhs/(bfloat)rhs);
    case expr_t::types::BOOL: return bint((int)(bool)lhs/(int)(bool)rhs);
    case expr_t::types::VECTOR:
      const size_t lsz=lhs.size();
      if(lsz!=rhs.size()) throw std::runtime_error("expr_t[]のサイズが異なります");
      const auto&lv=lhs.get<std::vector<expr_t>>(),&rv=rhs.get<std::vector<expr_t>>();
      std::vector<expr_t> ret(lsz);
      for(size_t i=0;i<lsz;++i) ret[i]=lv[i]/rv[i];
      return ret;
  }
  return expr_t();
}