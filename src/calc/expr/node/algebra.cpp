#include "../node.hpp"
namespace CALC{namespace EXPR{

expr_t algebra(tokenize&tok) noexcept(false) {
  expr_t lhs=term(tok);
  while(tok.top().type==token_t::SYMBOL
    &&(tok.top().token=="+" || tok.top().token=="-")
  ){
    std::string_view op=tok.top().token;
    tok.next_token();
    expr_t rhs=term(tok);
    if(op=="+") lhs+=std::move(rhs);
    else lhs-=std::move(rhs);
  }
  return lhs;
}

expr_t term(tokenize&tok) noexcept(false) {
  expr_t lhs = factor(tok);
  while(tok.top().type!=token_t::EMPTY){
    if(std::holds_alternative<bool>(lhs))
      lhs=expr_t(bint((int)std::get<bool>(lhs)));
    if(tok.top().type==token_t::SYMBOL){
      const std::string_view op=tok.top().token;
      if(op!="*"&&op!="%"&&op[0]!='/') break;
      tok.next_token();
      expr_t rhs=factor(tok);
      if(std::holds_alternative<bool>(rhs)) rhs=expr_t(bint((int)std::get<bool>(rhs)));
      if(op=="*")
        lhs*=std::move(rhs);
      else if(op=="%"){
        if(std::holds_alternative<bint>(lhs)&&std::holds_alternative<bint>(rhs)){
          if(std::get<bint>(rhs)==0){
            std::cerr<<"ゼロ除算はできません"<<std::endl;
            exit(EXIT_FAILURE);
          }
          std::get<bint>(lhs)/=std::get<bint>(rhs);
        }else{
          std::cerr<<"整数以外の型では剰余演算はできません"<<std::endl;
          exit(EXIT_FAILURE);
        }
      }else if(op=="/")
        lhs/=std::move(rhs);
      else if(op=="//"){
        bool Z=false;
        if(std::holds_alternative<bint>(rhs)){
          if(std::get<bint>(rhs)==0) Z=true;
        }else if(std::get<bfloat>(rhs)==0) Z=true;
        if(Z){
          std::cerr<<"ゼロ除算はできません"<<std::endl;
          exit(EXIT_FAILURE);
        }
        bool L=std::holds_alternative<bint>(lhs);
        bool R=std::holds_alternative<bint>(rhs);
        if(L&&R){
          bint q,r;
          divide_qr(std::get<bint>(lhs),std::get<bint>(rhs),q,r);
          if(Z=(r==0)) std::get<bint>(lhs)=q;
        }
        if(!Z){
          if(L)
            lhs=expr_t(static_cast<bfloat>(std::get<bint>(lhs))
              /(R?static_cast<bfloat>(std::get<bint>(rhs)):std::get<bfloat>(rhs)));
          else
            lhs/=R?static_cast<bfloat>(std::get<bint>(rhs)):std::get<bfloat>(rhs);
        }
      }
    }else
      try{ lhs*=factor(tok);
      }catch(const except&e){
        std::cerr<<"式の区切りが怪しい"<<std::endl;
      }
  }
  return lhs;
}

expr_t factor(tokenize&tok) noexcept(false) {
  expr_t lhs=atom(tok);
  std::vector<expr_t> rhs; // べき乗は右結合
  while(tok.top().type!=token_t::EMPTY){
    if(std::holds_alternative<bool>(lhs))
      lhs=expr_t(bint((int)std::get<bool>(lhs)));
    if(tok.top().type!=token_t::SYMBOL)break;
    if(tok.top().token=="^"||tok.top().token=="**"){
      tok.next_token();
      expr_t value=expr(tok);
      if(std::holds_alternative<bool>(value))
        value=expr_t(bint((int)std::get<bool>(value)));
      rhs.push_back(std::move(value));
    }else break;
  }
  if(rhs.size()==0) return lhs;
  while(rhs.size()>1){
    expr_t value=pow(rhs[rhs.size()-2],rhs.back());
    rhs.pop_back();
    rhs.back()=move(value);
  }
  return pow(lhs,rhs[0]);
}
}}//namespace EXPR}CALC}