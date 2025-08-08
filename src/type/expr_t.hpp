#pragma once
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <string>
#include <variant>
namespace mp=boost::multiprecision;
using bint=mp::cpp_int;
using bfloat=mp::cpp_dec_float_50;

class expr_t{
  std::variant<bint,bfloat,bool,std::vector<expr_t>> value;
  public:
  enum class types{
    BINT, BFLOAT, BOOL, VECTOR
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
  private:
  static types common_type(const expr_t&lhs,const expr_t&rhs);
};

template<expr_t::types T>
inline bool expr_t::is()const{ return type()==T; }

template<class T>
inline T& expr_t::get(){ return std::get<T>(value); }

template<class T>
inline const T& expr_t::get()const{ return std::get<T>(value); }