#include "type/expr_t.hpp"

static bool is_scalar_type(expr_t::types t) {
  return t == expr_t::types::BINT || t == expr_t::types::BFLOAT ||
         t == expr_t::types::COMPLEX || t == expr_t::types::BOOL;
}

static std::string type_name(const expr_t& e) {
  auto t = e.type();
  switch(t) {
    case expr_t::types::VOID: return "VOID";
    case expr_t::types::BINT: return "BINT";
    case expr_t::types::BFLOAT: return "BFLOAT";
    case expr_t::types::BOOL: return "BOOL";
    case expr_t::types::COMPLEX: return "COMPLEX";
    case expr_t::types::MATRIX: return "MATRIX";
    case expr_t::types::RANGE: return "RANGE";
    case expr_t::types::STRING: return "STRING";
    case expr_t::types::FUNCTION: {
      auto l = e.get<std::shared_ptr<LambdaFunc>>();
      std::string args_str = "(";
      for(size_t i=0; i<l->args.size(); ++i) {
        args_str += l->args[i];
        if(i + 1 < l->args.size()) args_str += ", ";
      }
      args_str += ")";
      return "FUNCTION" + args_str;
    }
  }
  return "UNKNOWN";
}


expr_t::types expr_t::common_type(const expr_t&lhs,const expr_t&rhs){
  types lt=lhs.type(),rt=rhs.type();
  if(lt==rt) return lt;
  if(lt==types::VOID || rt==types::VOID) throw std::runtime_error("void型との演算はできません");
  if(lt==types::COMPLEX || rt==types::COMPLEX) {
    if((lt==types::BFLOAT||lt==types::BINT||lt==types::BOOL) || (rt==types::BFLOAT||rt==types::BINT||rt==types::BOOL))
      return types::COMPLEX;
  }
  if(lt==types::BFLOAT||rt==types::BFLOAT) {
    if((lt==types::BINT||lt==types::BOOL) || (rt==types::BINT||rt==types::BOOL))
      return types::BFLOAT;
  }
  if(lt==types::BINT||rt==types::BINT) {
    if(lt==types::BOOL || rt==types::BOOL)
      return types::BINT;
  }
  throw std::runtime_error("型が一致しないため演算できません: " + type_name(lhs) + " と " + type_name(rhs));
}

expr_t::operator bool()const{
  switch(type()){
    case types::VOID: return false;
    case types::BINT: return std::get<bint>(value)!=0;
    case types::BFLOAT: return std::get<bfloat>(value)!=0;
    case types::BOOL: return std::get<bool>(value);
    case types::COMPLEX: {
      auto c = std::get<bcomplex>(value);
      return c.real() != 0 || c.imag() != 0;
    }
    case types::STRING: return !std::get<std::string>(value).empty();
    default: throw std::runtime_error("無効な型変換 -> bool");
  }
  return false;
}

expr_t::operator bint()const{
  switch(type()){
    case types::BINT: return std::get<bint>(value);
    case types::BFLOAT: return static_cast<bint>(std::get<bfloat>(value));
    case types::BOOL: return std::get<bool>(value);
    case types::COMPLEX: return static_cast<bint>(std::get<bcomplex>(value).real());
    default: throw std::runtime_error("無効な型変換 -> bint");
  }
  return 0;
}

expr_t::operator bfloat()const{
  switch(type()){
    case types::BINT: return static_cast<bfloat>(std::get<bint>(value));
    case types::BFLOAT: return std::get<bfloat>(value);
    case types::BOOL: return std::get<bool>(value);
    case types::COMPLEX: return std::get<bcomplex>(value).real();
    default: throw std::runtime_error("無効な型変換 -> bfloat");
  }
  return 0;
}

expr_t::operator bcomplex()const{
  switch(type()){
    case types::BINT: return bcomplex(static_cast<bfloat>(std::get<bint>(value)), 0);
    case types::BFLOAT: return bcomplex(std::get<bfloat>(value), 0);
    case types::BOOL: return bcomplex(std::get<bool>(value) ? 1 : 0, 0);
    case types::COMPLEX: return std::get<bcomplex>(value);
    default: throw std::runtime_error("無効な型変換 -> bcomplex");
  }
  return bcomplex(0, 0);
}

expr_t::types expr_t::type()const{
  return static_cast<types>(value.index());
}

size_t expr_t::size()const{
  if(type()==types::MATRIX) return get<std::shared_ptr<Matrix>>()->data.size();
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

std::shared_ptr<Matrix> invert_matrix(const std::shared_ptr<Matrix>& m){
  if(m->rows != m->cols) throw std::runtime_error("正方行列でなければ逆行列は定義されません");
  size_t n = m->rows;
  std::vector<std::vector<expr_t>> mat(n, std::vector<expr_t>(2 * n, bint(0)));
  for(size_t i=0; i<n; ++i){
    for(size_t j=0; j<n; ++j) mat[i][j] = m->data[i * n + j];
    mat[i][n + i] = bint(1);
  }
  for(size_t i=0; i<n; ++i){
    size_t pivot = i;
    for(size_t j=i+1; j<n; ++j){
      if(mat[j][i] != expr_t(bint(0)) && mat[pivot][i] == expr_t(bint(0))) {
        pivot = j;
      }
    }
    if(mat[pivot][i] == expr_t(bint(0))) {
      std::string msg = "行列は特異行列（正則でない）ため逆行列が存在しません (i=" + std::to_string(i) + ", pivot=" + std::to_string(pivot) + ")";
      throw std::runtime_error(msg);
    }
    if(pivot != i) std::swap(mat[i], mat[pivot]);
    expr_t divisor = mat[i][i];
    for(size_t j=i; j<2*n; ++j) mat[i][j] = mat[i][j] / divisor;
    for(size_t j=0; j<n; ++j){
      if(j != i){
        expr_t factor = mat[j][i];
        for(size_t k=i; k<2*n; ++k)
          mat[j][k] = mat[j][k] - factor * mat[i][k];
      }
    }
  }
  auto ret = std::make_shared<Matrix>();
  ret->rows = n;
  ret->cols = n;
  ret->data.resize(n * n);
  for(size_t i=0; i<n; ++i){
    for(size_t j=0; j<n; ++j){
      ret->data[i * n + j] = mat[i][n + j];
    }
  }
  return ret;
}

expr_t matrix_pow(const std::shared_ptr<Matrix>& m, bint k){
  if(m->rows != m->cols) throw std::runtime_error("べき乗は正方行列でのみ定義されます");
  if(k < 0){
    if(k == -1) return invert_matrix(m);
    throw std::runtime_error("行列のべき乗に負の数は指定できません (-1 を除く)");
  }
  if(k == 0){
    auto ret = std::make_shared<Matrix>();
    ret->rows = m->rows;
    ret->cols = m->cols;
    ret->data.resize(m->rows * m->cols, bint(0));
    for(size_t i=0; i<m->rows; ++i) ret->data[i * m->rows + i] = bint(1);
    return ret;
  }
  auto base = m;
  auto result = std::make_shared<Matrix>();
  result->rows = m->rows;
  result->cols = m->cols;
  result->data.resize(m->rows * m->cols, bint(0));
  for(size_t i=0; i<m->rows; ++i) result->data[i * m->rows + i] = bint(1);
  bint exponent = k;
  while(exponent > 0){
    if(exponent % 2 == 1){
      result = (expr_t(result) * expr_t(base)).get<std::shared_ptr<Matrix>>();
    }
    base = (expr_t(base) * expr_t(base)).get<std::shared_ptr<Matrix>>();
    exponent /= 2;
  }
  return result;
}

expr_t expr_t::pow(const expr_t&rhs){
  if(is<types::MATRIX>() && rhs.is<types::BINT>()){
    return matrix_pow(get<std::shared_ptr<Matrix>>(), rhs.get<bint>());
  }
  if(is<types::COMPLEX>() || rhs.is<types::COMPLEX>()){
    return std::pow((bcomplex)*this, (bcomplex)rhs);
  }
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
  auto lt = lhs.type(), rt = rhs.type();
  if(lt == expr_t::types::MATRIX && rt == expr_t::types::MATRIX){
    auto lm = lhs.get<std::shared_ptr<Matrix>>();
    auto rm = rhs.get<std::shared_ptr<Matrix>>();
    if(lm->rows != rm->rows || lm->cols != rm->cols) return false;
    for(size_t i=0; i<lm->data.size(); ++i){
      if(lm->data[i] != rm->data[i]) return false;
    }
    return true;
  }
  if((lt == expr_t::types::MATRIX && is_scalar_type(rt)) || (is_scalar_type(lt) && rt == expr_t::types::MATRIX)){
    throw std::runtime_error("行列とスカラーの比較は定義されません");
  }
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT:
      return (bint)lhs==(bint)rhs;
    case expr_t::types::BFLOAT:
      return (bfloat)lhs==(bfloat)rhs;
    case expr_t::types::BOOL:
      return (bool)lhs==(bool)rhs;
    case expr_t::types::COMPLEX:
      return (bcomplex)lhs==(bcomplex)rhs;
  }
  return false;
}

bool operator!=(const expr_t&lhs,const expr_t&rhs){
  return !(lhs==rhs);
}

bool operator<(const expr_t&lhs,const expr_t&rhs){
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT: return (bint)lhs<(bint)rhs;
    case expr_t::types::BFLOAT: return (bfloat)lhs<(bfloat)rhs;
    case expr_t::types::BOOL: return (bool)lhs<(bool)rhs;
    case expr_t::types::COMPLEX: throw std::runtime_error("複素数の大小比較は定義されません");
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
    case types::COMPLEX: return -(bcomplex)*this;
    case types::MATRIX: {
      auto m = get<std::shared_ptr<Matrix>>();
      auto ret = std::make_shared<Matrix>();
      ret->rows = m->rows;
      ret->cols = m->cols;
      ret->data.resize(m->data.size());
      for(size_t i=0; i<m->data.size(); ++i) ret->data[i] = -m->data[i];
      return ret;
    }
  }
  return expr_t();
}

expr_t operator+(const expr_t&lhs,const expr_t&rhs){
  auto lt = lhs.type(), rt = rhs.type();
  if(lt == expr_t::types::MATRIX && rt == expr_t::types::MATRIX){
    auto lm = lhs.get<std::shared_ptr<Matrix>>();
    auto rm = rhs.get<std::shared_ptr<Matrix>>();
    if(lm->rows != rm->rows || lm->cols != rm->cols)
      throw std::runtime_error("行列のサイズが一致しません");
    auto ret = std::make_shared<Matrix>();
    ret->rows = lm->rows;
    ret->cols = lm->cols;
    ret->data.resize(lm->data.size());
    for(size_t i=0; i<lm->data.size(); ++i)
      ret->data[i] = lm->data[i] + rm->data[i];
    return ret;
  }
  if(lt == expr_t::types::MATRIX && is_scalar_type(rt)){
    auto lm = lhs.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = lm->rows;
    ret->cols = lm->cols;
    ret->data.resize(lm->data.size());
    for(size_t i=0; i<lm->data.size(); ++i)
      ret->data[i] = lm->data[i] + rhs;
    return ret;
  }
  if(is_scalar_type(lt) && rt == expr_t::types::MATRIX){
    auto rm = rhs.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = rm->rows;
    ret->cols = rm->cols;
    ret->data.resize(rm->data.size());
    for(size_t i=0; i<rm->data.size(); ++i)
      ret->data[i] = lhs + rm->data[i];
    return ret;
  }
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT: return (bint)lhs+(bint)rhs;
    case expr_t::types::BFLOAT: return (bfloat)lhs+(bfloat)rhs;
    case expr_t::types::BOOL: return bint((int)(bool)lhs+(int)(bool)rhs);
    case expr_t::types::COMPLEX: return (bcomplex)lhs+(bcomplex)rhs;
  }
  return expr_t();
}

expr_t operator-(const expr_t&lhs,const expr_t&rhs){
  auto lt = lhs.type(), rt = rhs.type();
  if(lt == expr_t::types::MATRIX && rt == expr_t::types::MATRIX){
    auto lm = lhs.get<std::shared_ptr<Matrix>>();
    auto rm = rhs.get<std::shared_ptr<Matrix>>();
    if(lm->rows != rm->rows || lm->cols != rm->cols)
      throw std::runtime_error("行列のサイズが一致しません");
    auto ret = std::make_shared<Matrix>();
    ret->rows = lm->rows;
    ret->cols = lm->cols;
    ret->data.resize(lm->data.size());
    for(size_t i=0; i<lm->data.size(); ++i)
      ret->data[i] = lm->data[i] - rm->data[i];
    return ret;
  }
  if(lt == expr_t::types::MATRIX && is_scalar_type(rt)){
    auto lm = lhs.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = lm->rows;
    ret->cols = lm->cols;
    ret->data.resize(lm->data.size());
    for(size_t i=0; i<lm->data.size(); ++i)
      ret->data[i] = lm->data[i] - rhs;
    return ret;
  }
  if(is_scalar_type(lt) && rt == expr_t::types::MATRIX){
    auto rm = rhs.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = rm->rows;
    ret->cols = rm->cols;
    ret->data.resize(rm->data.size());
    for(size_t i=0; i<rm->data.size(); ++i)
      ret->data[i] = lhs - rm->data[i];
    return ret;
  }
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT: return (bint)lhs-(bint)rhs;
    case expr_t::types::BFLOAT: return (bfloat)lhs-(bfloat)rhs;
    case expr_t::types::BOOL: return bint((int)(bool)lhs-(int)(bool)rhs);
    case expr_t::types::COMPLEX: return (bcomplex)lhs-(bcomplex)rhs;
  }
  return expr_t();
}

expr_t operator*(const expr_t&lhs,const expr_t&rhs){
  auto lt = lhs.type(), rt = rhs.type();
  if(lt == expr_t::types::MATRIX && rt == expr_t::types::MATRIX){
    auto lm = lhs.get<std::shared_ptr<Matrix>>();
    auto rm = rhs.get<std::shared_ptr<Matrix>>();
    if(lm->cols != rm->rows)
      throw std::runtime_error("行列の積が定義されません");
    auto ret = std::make_shared<Matrix>();
    ret->rows = lm->rows;
    ret->cols = rm->cols;
    ret->data.resize(ret->rows * ret->cols, bint(0));
    for(size_t i=0; i<ret->rows; ++i){
      for(size_t j=0; j<ret->cols; ++j){
        expr_t sum = bint(0);
        for(size_t k=0; k<lm->cols; ++k)
          sum = sum + lm->data[i * lm->cols + k] * rm->data[k * rm->cols + j];
        ret->data[i * ret->cols + j] = sum;
      }
    }
    return ret;
  }
  if(lt == expr_t::types::MATRIX && is_scalar_type(rt)){
    auto lm = lhs.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = lm->rows;
    ret->cols = lm->cols;
    ret->data.resize(lm->data.size());
    for(size_t i=0; i<lm->data.size(); ++i)
      ret->data[i] = lm->data[i] * rhs;
    return ret;
  }
  if(is_scalar_type(lt) && rt == expr_t::types::MATRIX){
    auto rm = rhs.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = rm->rows;
    ret->cols = rm->cols;
    ret->data.resize(rm->data.size());
    for(size_t i=0; i<rm->data.size(); ++i)
      ret->data[i] = lhs * rm->data[i];
    return ret;
  }
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT: return (bint)lhs*(bint)rhs;
    case expr_t::types::BFLOAT: return (bfloat)lhs*(bfloat)rhs;
    case expr_t::types::BOOL: return (bool)lhs&&(bool)rhs;
    case expr_t::types::COMPLEX: return (bcomplex)lhs*(bcomplex)rhs;
  }
  return expr_t();
}

expr_t operator/(const expr_t&lhs,const expr_t&rhs){
  auto lt = lhs.type(), rt = rhs.type();
  if(lt == expr_t::types::MATRIX && rt == expr_t::types::MATRIX){
    auto lm = lhs.get<std::shared_ptr<Matrix>>();
    auto rm = rhs.get<std::shared_ptr<Matrix>>();
    auto rm_inv = invert_matrix(rm);
    return lhs * expr_t(rm_inv);
  }
  if(lt == expr_t::types::MATRIX && is_scalar_type(rt)){
    if(rhs == expr_t(bint(0))) {
      throw std::runtime_error("ゼロ除算エラー");
    }
    auto lm = lhs.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = lm->rows;
    ret->cols = lm->cols;
    ret->data.resize(lm->data.size());
    for(size_t i=0; i<lm->data.size(); ++i)
      ret->data[i] = lm->data[i] / rhs;
    return ret;
  }
  if(is_scalar_type(lt) && rt == expr_t::types::MATRIX){
    auto rm = rhs.get<std::shared_ptr<Matrix>>();
    auto rm_inv = invert_matrix(rm);
    return lhs * expr_t(rm_inv);
  }
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT:{
      bint q,r;
      mp::divide_qr((bint)lhs,(bint)rhs,q,r);
      if(r==0) return q;
      return (bfloat)q+(bfloat)r/(bfloat)rhs;
    }
    case expr_t::types::BFLOAT: return (bfloat)lhs/(bfloat)rhs;
    case expr_t::types::BOOL: return bint((int)(bool)lhs/(int)(bool)rhs);
    case expr_t::types::COMPLEX: return (bcomplex)lhs/(bcomplex)rhs;
  }
  return expr_t();
}

expr_t operator%(const expr_t&lhs,const expr_t&rhs){
  auto lt = lhs.type(), rt = rhs.type();
  if(lt == expr_t::types::MATRIX && is_scalar_type(rt)){
    if(rhs == expr_t(bint(0))) {
      throw std::runtime_error("剰余演算のゼロ除算エラー");
    }
    auto lm = lhs.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = lm->rows;
    ret->cols = lm->cols;
    ret->data.resize(lm->data.size());
    for(size_t i=0; i<lm->data.size(); ++i)
      ret->data[i] = lm->data[i] % rhs;
    return ret;
  }
  if(is_scalar_type(lt) && rt == expr_t::types::MATRIX){
    throw std::runtime_error("スカラーに対する行列の剰余算は定義されません");
  }
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT: return (bint)lhs%(bint)rhs;
    case expr_t::types::BFLOAT: throw std::runtime_error("浮動小数点型の剰余演算は禁止");
    case expr_t::types::BOOL: return bint((int)(bool)lhs%(int)(bool)rhs);
    case expr_t::types::COMPLEX: throw std::runtime_error("複素数型の剰余演算は禁止");
  }
  return expr_t();
}

expr_t idiv(const expr_t&lhs,const expr_t&rhs){
  switch(expr_t::common_type(lhs,rhs)){
    case expr_t::types::BINT: return (bint)lhs/(bint)rhs;
    case expr_t::types::BFLOAT: return mp::trunc((bfloat)lhs/(bfloat)rhs);
    case expr_t::types::BOOL: return bint((int)(bool)lhs/(int)(bool)rhs);
    case expr_t::types::COMPLEX: throw std::runtime_error("複素数型の割り切り除算は禁止");
  }
  return expr_t();
}

#include "ast/ast.hpp"
LambdaFunc::~LambdaFunc() {
  delete body;
}