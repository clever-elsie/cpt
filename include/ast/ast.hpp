#pragma once
#include <unordered_set>
#include <vector>
#include <json_v2.hpp>
#include "type/expr_t.hpp"
#include "type/map_var_fn.hpp"

namespace AST{

inline VAR_MAP<expr_t> var_map;
inline FN_MAP fn_map;
inline std::string input_buffer;

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
  virtual bool is_block_style() const { return false; }
  virtual json::value to_json() const = 0;
  protected:
  Nitem(size_t row,size_t col);
  Nitem() : row(0), col(0) {}
  size_t row = 0, col = 0;
};

struct Nstat:public Nitem{
  std::vector<Nitem*> items;
  std::vector<std::string> args,var_names;
  std::unordered_set<std::string,MAP_VAR_FN::StringHash,MAP_VAR_FN::StringEqual> args_set, var_names_set;
  Nstat();
  virtual expr_t get_value() override;
  expr_t evaluate(std::vector<expr_t>&&args);
  virtual json::value to_json() const override;
  ~Nstat();
};

class Nexpr:public Nitem{
  Nitem*lhs,*rhs,*ths; // [left | right | third] hand side
  id_t id;
  op_t op;
  public:
  Nexpr(size_t row,size_t col, op_t op, Nitem*lhs, Nitem*rhs=nullptr, Nitem*ths=nullptr);
  virtual expr_t get_value()override;
  virtual json::value to_json() const override;
  ~Nexpr();
  private:
  expr_t eval_uop();
  void eval_uop_fact(expr_t&value);

  expr_t eval_bop();
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
  virtual json::value to_json() const override;
  void move_to(Nstat*belong_to);
  std::string_view get_name()const;
};

class Nvar:public Nitem{
  std::string name;
  public:
  Nvar(size_t row,size_t col,std::string_view name);
  virtual expr_t get_value()override;
  virtual json::value to_json() const override;
  std::string_view get_name()const;
  ~Nvar()=default;
};

class Nliteral:public Nitem{
  expr_t value;
  public:
  Nliteral(size_t row,size_t col,expr_t&&value);
  virtual expr_t get_value()override;
  virtual json::value to_json() const override;
  ~Nliteral()=default;
};

class Nfn:public Nitem{
  friend class Npipeline;
  std::string name;
  std::vector<Nitem*> args;
  public:
  Nfn(size_t row,size_t col,std::string_view name,std::vector<Nitem*>&&args);
  virtual expr_t get_value()override;
  virtual json::value to_json() const override;
  ~Nfn();
  private:
  expr_t eval_reserved_fn();
};

class Nif : public Nitem {
  Nitem* cond;
  Nitem* then_expr;
  Nitem* else_expr;
public:
  Nif(size_t row, size_t col, Nitem* cond, Nitem* then_expr, Nitem* else_expr = nullptr);
  virtual expr_t get_value() override;
  virtual bool is_block_style() const override { return true; }
  virtual json::value to_json() const override;
  ~Nif();
};

class Nwhile : public Nitem {
  Nitem* cond;
  Nitem* body;
public:
  Nwhile(size_t row, size_t col, Nitem* cond, Nitem* body);
  virtual expr_t get_value() override;
  virtual bool is_block_style() const override { return true; }
  virtual json::value to_json() const override;
  ~Nwhile();
};

class Nfor : public Nitem {
  std::string var_name;
  Nitem* range_expr;
  Nitem* body;
public:
  Nfor(size_t row, size_t col, std::string_view var_name, Nitem* range_expr, Nitem* body);
  virtual expr_t get_value() override;
  virtual bool is_block_style() const override { return true; }
  virtual json::value to_json() const override;
  ~Nfor();
};

class Nlambda : public Nitem {
  std::vector<std::string> args;
  Nstat* body;
public:
  Nlambda(size_t row, size_t col, std::vector<std::string>&& args, Nstat* body);
  virtual expr_t get_value() override;
  virtual bool is_block_style() const override { return true; }
  virtual json::value to_json() const override;
  ~Nlambda();
};

class Npipeline : public Nitem {
  Nitem* lhs;
  Nitem* rhs;
public:
  Npipeline(size_t row, size_t col, Nitem* lhs, Nitem* rhs);
  virtual expr_t get_value() override;
  virtual json::value to_json() const override;
  ~Npipeline();
};

class Nns_resolve : public Nitem {
  std::string alias;
  std::string name;
public:
  Nns_resolve(size_t row, size_t col, std::string_view alias, std::string_view name);
  virtual expr_t get_value() override;
  virtual json::value to_json() const override;
  ~Nns_resolve() = default;
};

class Nrange : public Nitem {
  Nitem* start_expr;
  Nitem* end_expr;
  bool is_inclusive;
public:
  Nrange(size_t row, size_t col, Nitem* start_expr, Nitem* end_expr, bool is_inclusive);
  virtual expr_t get_value() override;
  virtual json::value to_json() const override;
  ~Nrange();
};


class Nsubscript : public Nitem {
  Nitem* var_expr;
  Nitem* index1;
  Nitem* index2; // nullptr if single index
public:
  Nsubscript(size_t row, size_t col, Nitem* var_expr, Nitem* index1, Nitem* index2 = nullptr);
  virtual expr_t get_value() override;
  expr_t get_reference();
  virtual json::value to_json() const override;
  ~Nsubscript();
};


class Nmatrix : public Nitem {
  size_t rows;
  size_t cols;
  std::vector<Nitem*> elements;
public:
  Nmatrix(size_t row,size_t col,size_t rows,size_t cols,std::vector<Nitem*>&& elements);
  virtual expr_t get_value() override;
  virtual json::value to_json() const override;
  ~Nmatrix();
};

}// namespace AST