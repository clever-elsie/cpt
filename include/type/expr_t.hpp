#pragma once
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <complex>
#include <unordered_map>

namespace mp=boost::multiprecision;
using bint=mp::cpp_int;
using bfloat=mp::cpp_dec_float_50;
using bcomplex=std::complex<bfloat>;

namespace AST{
struct Nstat;
}

struct Ref;

struct Matrix {
  size_t rows;
  size_t cols;
  std::vector<class expr_t> data;
  bool is_as_mat = false;

  std::vector<class expr_t> get_iterable_elements() const;
};

struct Range {
  bint start;
  bint end;
  bool is_inclusive;
};

struct LambdaFunc {
  std::vector<std::string> args;
  AST::Nstat* body;
  std::unordered_map<std::string, class expr_t> closure_env;
  ~LambdaFunc();
};

struct Ref {
  virtual class expr_t get_value() const = 0;
  virtual void set_value(const class expr_t& val) = 0;
  virtual ~Ref() = default;
};

struct VariableRef : public Ref {
  std::string var_name;
  virtual class expr_t get_value() const override;
  virtual void set_value(const class expr_t& val) override;
};

struct MatrixElementRef : public Ref {
  std::shared_ptr<Matrix> matrix;
  size_t index;
  virtual class expr_t get_value() const override;
  virtual void set_value(const class expr_t& val) override;
};

class expr_t{
  std::variant<
    std::monostate,               // VOID (0)
    bint,                         // BINT (1)
    bfloat,                       // BFLOAT (2)
    bool,                         // BOOL (3)
    bcomplex,                     // COMPLEX (4)
    std::shared_ptr<Matrix>,      // MATRIX (5)
    std::shared_ptr<Range>,       // RANGE (6)
    std::shared_ptr<LambdaFunc>,  // FUNCTION (7)
    std::string,                  // STRING (8)
    std::shared_ptr<Ref>          // REF (9)
  > value;
  public:
  enum class types{
    VOID = 0, BINT = 1, BFLOAT = 2, BOOL = 3, COMPLEX = 4,
    MATRIX = 5, RANGE = 6, FUNCTION = 7, STRING = 8, REF = 9
  };
  expr_t()=default;
  expr_t(const expr_t&other)=default;
  expr_t(expr_t&&other)=default;
  expr_t&operator=(const expr_t&other)=default;
  expr_t&operator=(expr_t&&other)=default;
  template<class T>
  requires (!std::same_as<expr_t,std::decay_t<T>>)
  expr_t(T&&value):value(std::forward<T>(value)){};
  types type()const;
  expr_t deref()const;
  template<types T> bool is()const;
  template<class T> T&get();
  template<class T> const T&get()const;
  size_t size()const;
  expr_t pow(const expr_t&rhs);
  friend bool operator&&(const expr_t&lhs,const expr_t&rhs);
  friend bool operator||(const expr_t&lhs,const expr_t&rhs);
  friend bool operator==(const expr_t&lhs,const expr_t&rhs);
  friend bool operator!=(const expr_t&lhs,const expr_t&rhs);
  friend bool operator<(const expr_t&lhs,const expr_t&rhs);
  friend bool operator<=(const expr_t&lhs,const expr_t&rhs);
  friend bool operator>(const expr_t&lhs,const expr_t&rhs);
  friend bool operator>=(const expr_t&lhs,const expr_t&rhs);
  bool operator!()const;
  expr_t operator-()const;
  friend expr_t operator+(const expr_t&lhs,const expr_t&rhs);
  friend expr_t operator-(const expr_t&lhs,const expr_t&rhs);
  friend expr_t operator*(const expr_t&lhs,const expr_t&rhs);
  friend expr_t operator/(const expr_t&lhs,const expr_t&rhs);
  friend expr_t operator%(const expr_t&lhs,const expr_t&rhs);
  friend expr_t idiv(const expr_t&lhs,const expr_t&rhs);
  operator bool()const;
  operator bint()const;
  operator bfloat()const;
  operator bcomplex()const;
  private:
  static types common_type(const expr_t&lhs,const expr_t&rhs);
};

template<expr_t::types T>
inline bool expr_t::is()const{ return type()==T; }

template<class T>
inline T& expr_t::get(){ return std::get<T>(value); }

template<class T>
inline const T& expr_t::get()const{ return std::get<T>(value); }