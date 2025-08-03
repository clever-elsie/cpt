#pragma once
#include <vector>
#include "../calc/type.hpp"
#include "../map_var_fn.hpp"

namespace CALC{ namespace AST{

inline VAR_MAP<expr_t> var_map;
inline FN_MAP fn_map;

enum class id_t{
  uop, bop, top,
  var, literal, fn, reserve
};

enum class op_t{
  NOP, NOT, NEG, FACT,
  ADD, SUB, MUL, POW,
  FDIV, IDIV, MOD,
  LOR, LAND, EQ, NE,
  LT, GT, LE, GE, BR,
};

constexpr id_t get_id(op_t op){
  assert(op!=op_t::NOP);
  switch(op){
    case op_t::NOT: // 論理否定
    case op_t::NEG: // 符号反転
    case op_t::FACT: return id_t::uop; // 階乗
    case op_t::BR: return id_t::top; // 条件分岐?:
    default: break; // それ以外は全部二項演算
  }
  return id_t::bop;
}

struct Nitem{
  virtual expr_t get_value()=0;
  virtual ~Nitem()=default;
  protected:
  Nitem(size_t row,size_t col):row(row),col(col){}
  Nitem()=default;
  size_t row,col;
};

struct Nstat{
  std::vector<Nitem*> items;
  Nstat()=default;
};

class Nexpr:public Nitem{
  Nexpr*lhs,*rhs,*ths; // [left | right | third] hand side
  id_t id;
  op_t op;
  public:
  Nexpr(size_t row,size_t col, op_t op, Nexpr*lhs, Nexpr*rhs=nullptr, Nexpr*ths=nullptr)
  :Nitem(row,col),lhs(lhs),rhs(rhs),ths(ths),op(op){
    id=get_id(op);
  }
  virtual expr_t get_value()override{
    switch(id){
      case id_t::uop: eval_uop(); break;
      case id_t::bop: eval_bop(); break;
      case id_t::top: eval_top(); break;
    }
  }
  private:
  expr_t eval_uop(){
    assert(lhs);
    expr_t value=lhs->get_value();
    switch(op){
      case op_t::NOT: eval_uop_not(value); break;
      case op_t::NEG: eval_uop_neg(value); break;
      case op_t::FACT: eval_uop_fact(value); break;
    }
    return value;
  }
  void eval_uop_not(expr_t&value){
    if(std::holds_alternative<bint>(value))
      value=std::get<bint>(value)!=0;
    else if(std::holds_alternative<bfloat>(value))
      value=std::get<bfloat>(value)!=0;
    if(std::holds_alternative<bool>(value))
      value=!std::get<bool>(value);
    else throw std::runtime_error("無効な論理否定");
  }
  void eval_uop_neg(expr_t&value){
    if(std::holds_alternative<bool>(value))
      value=bint((int)std::get<bool>(value));
    if(std::holds_alternative<bint>(value))
      value=-std::get<bint>(value);
    else if(std::holds_alternative<bfloat>(value))
      value=-std::get<bfloat>(value);
    else throw std::runtime_error("無効な符号反転");
  }
  void eval_uop_fact(expr_t&value){
    if(!std::holds_alternative<bint>(value))
      throw std::runtime_error("階乗は整数値でなければなりません");
    if(std::get<bint>(value)<0)
      throw std::runtime_error("階乗は負の整数に対し定義されていません");
    if(std::get<bint>(value)==0) value=bint(1);
    for(bint n=std::get<bint>(value);--n>=2;) std::get<bint>(value)*=n;
  }

  expr_t eval_bop(){
    assert(lhs&&rhs);
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
      case op_t::LOR: return lhs_value||rhs_value;
      case op_t::LAND: return lhs_value&&rhs_value;
      case op_t::EQ: return lhs_value==rhs_value;
      case op_t::NE: return lhs_value!=rhs_value;
      case op_t::LT: return lhs_value<rhs_value;
      case op_t::GT: return lhs_value>rhs_value;
      case op_t::LE: return lhs_value<=rhs_value;
      case op_t::GE: return lhs_value>=rhs_value;
    }
    return lhs_value;
  }
  void eval_bop_idiv(expr_t&lhs,expr_t&rhs){
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

  expr_t eval_top(){ // top=ternary operator
    assert(lhs&&rhs&&ths);
    // 現状?:のみ．そもそも三項演算子は他にあるのか？
    if(to_bool(lhs->get_value())) return rhs->get_value();
    return ths->get_value();
  }
};

class Ndecl:public Nitem{
  std::string_view name;
  Nitem*init;
  public:
  Ndecl(std::string_view name,Nitem*init):name(name),init(init){}
  virtual expr_t get_value()override{
    // 初期値を評価．変数テーブルに登録
  }
};

class Nreserve:public Nitem{
  std::string_view name;
  public:
  virtual expr_t get_value()override;
};

class Nvar:public Nitem{
  std::string_view name;
  public:
  virtual expr_t get_value()override{
    auto it=var_map.find(name);
    if(it==var_map.end()||it->second.empty())
      throw std::runtime_error("未定義の変数を参照しています");
  }
};

class Nliteral:public Nitem{
  expr_t value;
  public:
  virtual expr_t get_value()override{ return value; }
};

class Nfn:public Nitem{
  std::string_view name;
  std::vector<Nitem> args;
  public:
  virtual expr_t get_value()override;
};

} }// namespace AST}CALC}