#include "ast/ast.hpp"
#include <typeinfo>
#include <iostream>

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
  delete lhs;
  delete rhs;
  delete ths;
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
    case op_t::NOT: value=!value; break;
    case op_t::NEG: value=-value; break;
    case op_t::FACT: eval_uop_fact(value); break;
  }
  return value;
}

inline void Nexpr::eval_uop_fact(expr_t&value){
  if(!value.is<expr_t::types::BINT>()) {
    auto [row, col] = this->get_pos();
    throw std::runtime_error("階乗は整数値でなければなりません at [row=" + std::to_string(row) + ", col=" + std::to_string(col) + "]");
  }
  if(value.get<bint>()<0) {
    auto [row, col] = this->get_pos();
    throw std::runtime_error("階乗は負の整数に対し定義されていません at [row=" + std::to_string(row) + ", col=" + std::to_string(col) + "]");
  }
  if(value.get<bint>()==0) value=bint(1);
  for(bint n=value.get<bint>();--n>=2;) value.get<bint>()*=n;
}

expr_t Nexpr::eval_bop(){
  assert(lhs&&rhs);
  switch(op){ // 短絡評価
    case op_t::LOR:{
      if((bool)(lhs->get_value())) return true;
      return rhs->get_value();
    }break;
    case op_t::LAND:{
      if(!(lhs->get_value())) return false;
      return rhs->get_value();
    }break;
    case op_t::ASSIGN: return eval_bop_assign(lhs,rhs); break;
    default:break;
  }
  expr_t lhs_value=lhs->get_value();
  expr_t rhs_value=rhs->get_value();
  expr_t ret;
  switch(op){
    case op_t::ADD: ret=lhs_value+rhs_value; break;
    case op_t::SUB: ret=lhs_value-rhs_value; break;
    case op_t::MUL: ret=lhs_value*rhs_value; break;
    case op_t::POW: ret=lhs_value.pow(rhs_value); break;
    case op_t::FDIV: ret=lhs_value/rhs_value; break;
    case op_t::IDIV: ret=idiv(lhs_value,rhs_value); break;
    case op_t::MOD: ret=lhs_value%rhs_value; break;
  }
  return ret;
}

inline expr_t Nexpr::eval_bop_assign(Nitem*lhs,Nitem*rhs){
  expr_t value=rhs->get_value();
  
  auto sub_ptr = dynamic_cast<Nsubscript*>(lhs);
  if (sub_ptr != nullptr) {
    expr_t ref_val = sub_ptr->get_reference();
    if (ref_val.is<expr_t::types::REF>()) {
      ref_val.get<std::shared_ptr<Ref>>()->set_value(value);
      return value;
    } else if (ref_val.is<expr_t::types::MATRIX>()) {
      auto m_ref = ref_val.get<std::shared_ptr<Matrix>>();
      if (value.is<expr_t::types::MATRIX>()) {
        auto val_m = value.get<std::shared_ptr<Matrix>>();
        if (m_ref->rows != val_m->rows || m_ref->cols != val_m->cols) {
          throw std::runtime_error("代入する行列のサイズが一致しません");
        }
        for (size_t i = 0; i < m_ref->data.size(); ++i) {
          if (m_ref->data[i].is<expr_t::types::REF>()) {
            m_ref->data[i].get<std::shared_ptr<Ref>>()->set_value(val_m->data[i].deref());
          }
        }
      } else {
        for (size_t i = 0; i < m_ref->data.size(); ++i) {
          if (m_ref->data[i].is<expr_t::types::REF>()) {
            m_ref->data[i].get<std::shared_ptr<Ref>>()->set_value(value);
          }
        }
      }
      return value;
    }
    throw std::runtime_error("左辺値は参照可能である必要があります．");
  }
  
  auto ptr=dynamic_cast<Nvar*>(lhs);
  if(ptr==nullptr) throw std::runtime_error("左辺値の取得に失敗しました．");
  std::string_view name=ptr->get_name();
  auto itr=var_map.find(name);
  if(itr==var_map.end()||itr->second.empty())
    throw std::runtime_error(std::string(name)+"は未定義変数です");
    
  if (itr->second.back().is<expr_t::types::REF>()) {
    itr->second.back().get<std::shared_ptr<Ref>>()->set_value(value);
    return value;
  }
  
  return itr->second.back()=std::move(value);
}

expr_t Nexpr::eval_top(){
  if(op==op_t::BR){
    assert(lhs&&rhs&&ths);
    if((bool)(lhs->get_value())) return rhs->get_value();
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

static std::string_view op_to_str(op_t op) {
  switch(op) {
    case op_t::NOP: return "NOP";
    case op_t::NOT: return "NOT";
    case op_t::NEG: return "NEG";
    case op_t::FACT: return "FACT";
    case op_t::ASSIGN: return "ASSIGN";
    case op_t::ADD: return "ADD";
    case op_t::SUB: return "SUB";
    case op_t::MUL: return "MUL";
    case op_t::POW: return "POW";
    case op_t::FDIV: return "FDIV";
    case op_t::IDIV: return "IDIV";
    case op_t::MOD: return "MOD";
    case op_t::LOR: return "LOR";
    case op_t::LAND: return "LAND";
    case op_t::EQ: return "EQ";
    case op_t::NE: return "NE";
    case op_t::LT: return "LT";
    case op_t::GT: return "GT";
    case op_t::LE: return "LE";
    case op_t::GE: return "GE";
    case op_t::BR: return "BR";
  }
  return "UNKNOWN";
}

json::value Nexpr::to_json() const {
  json::value v;
  v["type"] = "expr";
  v["op"] = std::string(op_to_str(op));
  v["lhs"] = lhs ? lhs->to_json() : json::value();
  if (rhs) v["rhs"] = rhs->to_json();
  if (ths) v["ths"] = ths->to_json();
  v["row"] = (int64_t)row;
  v["col"] = (int64_t)col;
  return v;
}

} // namespace AST