#pragma once
#include <string>
#include <string_view>
#include <functional>
#include <unordered_map>
#include <vector>

namespace AST{
struct Nstat;
struct Nitem;
}

namespace MAP_VAR_FN{

using namespace std;
using sv=string_view;
using str=const string&;

struct StringHash{
  using is_transparent = void;
  size_t operator()(sv sv_)const noexcept{return hash<sv>{}(sv_);}
  size_t operator()(str s)const noexcept{return hash<sv>{}(s);}
};
struct StringEqual{
  using is_transparent=void;
  bool operator()(sv lhs,sv rhs)const noexcept{return lhs==rhs;}
  bool operator()(str lhs,sv rhs)const noexcept{return lhs==rhs;}
  bool operator()(sv lhs,str rhs)const noexcept{return lhs==rhs;}
  bool operator()(str lhs,str rhs)const noexcept{return lhs==rhs;}
};
}// namespace MAP_VAR_FN

template<class expr_t>
using VAR_MAP=std::unordered_map<
  std::string,
  std::vector<expr_t>,
  MAP_VAR_FN::StringHash,
  MAP_VAR_FN::StringEqual
>;

using FN_MAP=std::unordered_map<
  std::string,
  AST::Nstat*,
  MAP_VAR_FN::StringHash,
  MAP_VAR_FN::StringEqual
>;
