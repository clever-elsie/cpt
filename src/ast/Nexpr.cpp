#include "ast.hpp"

namespace AST{

constexpr static inline id_t get_id(op_t op){
  if(op==op_t::NOP) throw std::runtime_error("無効な演算子 (op==NOP)");
  switch(op){
    case op_t::NOT: case op_t::NEG:
    case op_t::FACT: return id_t::uop; // 階乗
    case op_t::LOR: case op_t::LAND: // これらはtopにしなくても短絡評価できる
    case op_t::ADD: case op_t::SUB:
    case op_t::MUL: case op_t::POW:
    case op_t::FDIV: case op_t::IDIV:
    case op_t::MOD:
    case op_t::ASSIGN: return id_t::bop;
    case op_t::EQ: // 比較演算子は連鎖評価するためにtopとして扱う
    case op_t::NE: // 1<x<2のときxを一度の評価でいいようにthsに次の式を入れる
    case op_t::LT: // thsで参照された式では前項のrhsをlhsとして扱う
    case op_t::GT: case op_t::LE: case op_t::GE:
    case op_t::BR: return id_t::top; // 条件分岐?:
  }
  throw std::runtime_error("無効な演算子 (switch default)");
  return id_t::uop; // dead code for no return error of compiler
}

Nexpr::Nexpr(size_t row, size_t col, op_t op,
  Nitem*lhs, Nitem*rhs, Nitem*ths)
:Nitem(row,col),lhs(lhs),rhs(rhs),ths(ths),op(op){
  try{
    id=get_id(op);
  }catch(const std::runtime_error&e){
    std::cerr<<"Nexpr::Nexpr(row="<<row<<", col="<<col<<": "<<e.what()<<std::endl;
    throw;
  }
}

Nexpr::~Nexpr(){
  delete lhs,rhs,ths;
}

expr_t Nexpr::get_value(){
  switch(id){
    case id_t::uop: return eval_uop();
    case id_t::bop: return eval_bop();
    case id_t::top: return eval_top();
    default: {
      auto [row, col] = this->get_pos();
      throw std::runtime_error("無効な演算子 at [row=" + std::to_string(row) + ", col=" + std::to_string(col) + "]");
    }
  }
  return expr_t(); // dead code
}

expr_t Nexpr::eval_uop(){
  assert(lhs);
  expr_t value=lhs->get_value();
  switch(op){
    case op_t::NOT: eval_uop_not(value); break;
    case op_t::NEG: eval_uop_neg(value); break;
    case op_t::FACT: eval_uop_fact(value); break;
  }
  return value;
}

inline void Nexpr::eval_uop_not(expr_t&value){
  if(std::holds_alternative<bint>(value))
    value=std::get<bint>(value)!=0;
  else if(std::holds_alternative<bfloat>(value))
    value=std::get<bfloat>(value)!=0;
  if(std::holds_alternative<bool>(value))
    value=!std::get<bool>(value);
  else {
    auto [row, col] = this->get_pos();
    throw std::runtime_error("無効な論理否定 at [row=" + std::to_string(row) + ", col=" + std::to_string(col) + "]");
  }
}

inline void Nexpr::eval_uop_neg(expr_t&value){
  if(std::holds_alternative<bool>(value))
    value=bint((int)std::get<bool>(value));
  if(std::holds_alternative<bint>(value))
    value=-std::get<bint>(value);
  else if(std::holds_alternative<bfloat>(value))
    value=-std::get<bfloat>(value);
  else {
    auto [row, col] = this->get_pos();
    throw std::runtime_error("無効な符号反転 at [row=" + std::to_string(row) + ", col=" + std::to_string(col) + "]");
  }
}

inline void Nexpr::eval_uop_fact(expr_t&value){
  if(!std::holds_alternative<bint>(value)) {
    auto [row, col] = this->get_pos();
    throw std::runtime_error("階乗は整数値でなければなりません at [row=" + std::to_string(row) + ", col=" + std::to_string(col) + "]");
  }
  if(std::get<bint>(value)<0) {
    auto [row, col] = this->get_pos();
    throw std::runtime_error("階乗は負の整数に対し定義されていません at [row=" + std::to_string(row) + ", col=" + std::to_string(col) + "]");
  }
  if(std::get<bint>(value)==0) value=bint(1);
  for(bint n=std::get<bint>(value);--n>=2;) std::get<bint>(value)*=n;
}

expr_t Nexpr::eval_bop(){
  assert(lhs&&rhs);
  switch(op){ // 短絡評価
    case op_t::LOR:{
      if(to_bool(lhs->get_value())) return true;
      return rhs->get_value();
    }break;
    case op_t::LAND:{
      if(!to_bool(lhs->get_value())) return false;
      return rhs->get_value();
    }break;
    case op_t::ASSIGN: return eval_bop_assign(lhs,rhs); break;
    default:break;
  }
  expr_t lhs_value=lhs->get_value();
  expr_t rhs_value=rhs->get_value();
  switch(op){
    case op_t::ADD: lhs_value+=rhs_value; break;
    case op_t::SUB: lhs_value-=rhs_value; break;
    case op_t::MUL: lhs_value*=rhs_value; break;
    case op_t::POW: lhs_value=pow(lhs_value,rhs_value); break;
    case op_t::FDIV: lhs_value/=rhs_value; break;
    case op_t::IDIV: eval_bop_idiv(lhs_value,rhs_value); break;
    case op_t::MOD: lhs_value%=rhs_value; break;
  }
  return lhs_value;
}

void Nexpr::eval_bop_idiv(expr_t&lhs,expr_t&rhs){
  bool Z=false;
  if(std::holds_alternative<bool>(lhs))
    lhs=bint((int)std::get<bool>(lhs));
  if(std::holds_alternative<bool>(rhs))
    rhs=bint((int)std::get<bool>(rhs));
  if(std::holds_alternative<bint>(rhs)){
    if(std::get<bint>(rhs)==0) Z=true;
  }else if(std::get<bfloat>(rhs)==0) Z=true;
  if(Z) throw std::runtime_error("ゼロ除算はできません");
  bool L=std::holds_alternative<bint>(lhs);
  bool R=std::holds_alternative<bint>(rhs);
  if(L&&R){ // 両方整数の時はmp::divide_qrが使える
    bint q,r;
    divide_qr(std::get<bint>(lhs),std::get<bint>(rhs),q,r);
    lhs=std::move(q);
  }else{
    if(L) lhs=(bfloat)std::get<bint>(lhs)/std::get<bfloat>(rhs);
    else lhs/=R?(bfloat)std::get<bint>(rhs):std::get<bfloat>(rhs);
  }
}

inline expr_t Nexpr::eval_bop_assign(Nitem*lhs,Nitem*rhs){
  expr_t value=rhs->get_value();
  auto ptr=dynamic_cast<Nvar*>(lhs);
  if(ptr==nullptr) throw std::runtime_error("左辺値の取得に失敗しました．");
  std::string_view name=ptr->get_name();
  auto itr=var_map.find(name);
  if(itr==var_map.end()||itr->second.empty())
    throw std::runtime_error(std::string(name)+"は未定義変数です");
  return itr->second.back()=std::move(value);
}

expr_t Nexpr::eval_top(){
  if(op==op_t::BR){
    assert(lhs&&rhs&&ths);
    if(to_bool(lhs->get_value())) return rhs->get_value();
    return ths->get_value();
  }
  // 比較
  Nexpr*itr=this;
  expr_t lhs_value=lhs->get_value();
  while(itr!=nullptr){
    expr_t rhs_value=itr->rhs->get_value();
    switch(itr->op){
      case op_t::EQ:
        if(lhs_value!=rhs_value) return false;
        break;
      case op_t::NE:
        if(lhs_value==rhs_value) return false;
        break;
      case op_t::LT:
        if(lhs_value>=rhs_value) return false;
        break;
      case op_t::GT:
        if(lhs_value<=rhs_value) return false;
        break;
      case op_t::LE:
        if(lhs_value>rhs_value) return false;
        break;
      case op_t::GE:
        if(lhs_value<rhs_value) return false;
        break;
    }
    lhs_value=std::move(rhs_value);
    itr=dynamic_cast<Nexpr*>(itr->ths);
  }
  return true;
}


} // namespace AST