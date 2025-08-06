#pragma once
#include <unordered_set>
#include <vector>
#include "../type/expr_t.hpp"
#include "../type/map_var_fn.hpp"

namespace AST{

inline VAR_MAP<expr_t> var_map;
inline FN_MAP fn_map;

enum class id_t{
  uop, bop, top,
  var, literal, fn, reserve
};

enum class op_t{
  NOP, NOT, NEG, FACT,
  ASSIGN, ADD, SUB, MUL, POW,
  FDIV, IDIV, MOD,
  LOR, LAND, EQ, NE,
  LT, GT, LE, GE, BR,
};

struct Nitem{
  virtual expr_t get_value()=0;
  virtual ~Nitem()=default;
  void error_exit(std::string_view msg);
  std::pair<size_t,size_t> get_pos()const;
  protected:
  Nitem(size_t row,size_t col);
  Nitem()=default;
  size_t row,col;
};

struct Nstat{
  std::vector<Nitem*> items;
  std::vector<std::string> args,var_names;
  std::unordered_set<std::string,MAP_VAR_FN::StringHash,MAP_VAR_FN::StringEqual> args_set, var_names_set;
  Nstat()=default;
  expr_t evaluate(std::vector<expr_t>&&args);
  ~Nstat();
};

class Nexpr:public Nitem{
  Nitem*lhs,*rhs,*ths; // [left | right | third] hand side
  id_t id;
  op_t op;
  public:
  Nexpr(size_t row,size_t col, op_t op, Nitem*lhs, Nitem*rhs=nullptr, Nitem*ths=nullptr);
  virtual expr_t get_value()override;
  ~Nexpr();
  private:
  expr_t eval_uop();
  void eval_uop_not(expr_t&value);
  void eval_uop_neg(expr_t&value);
  void eval_uop_fact(expr_t&value);

  expr_t eval_bop();
  void eval_bop_idiv(expr_t&lhs,expr_t&rhs);
  expr_t eval_bop_assign(Nitem*lhs,Nitem*rhs);

  expr_t eval_top(); // top=ternary operator
};

class Ndecl:public Nitem{
  std::string name;
  Nitem*init;
  bool is_local_first;
  Nstat*belong_to;
  public:
  Ndecl(std::string_view name,Nitem*init,Nstat*belong_to);
  ~Ndecl();
  virtual expr_t get_value()override;
  void move_to(Nstat*belong_to);
  std::string_view get_name()const;
};

class Nvar:public Nitem{
  std::string name;
  public:
  Nvar(size_t row,size_t col,std::string_view name);
  virtual expr_t get_value()override;
  std::string_view get_name()const;
  ~Nvar()=default;
};

class Nliteral:public Nitem{
  expr_t value;
  public:
  Nliteral(size_t row,size_t col,expr_t&&value);
  virtual expr_t get_value()override;
  ~Nliteral()=default;
};

class Nfn:public Nitem{
  std::string name;
  std::vector<Nitem*> args;
  public:
  Nfn(size_t row,size_t col,std::string_view name,std::vector<Nitem*>&&args);
  virtual expr_t get_value()override;
  ~Nfn();
  private:
  expr_t eval_reserved_fn();
};

}// namespace AST