#include "type/expr_t.hpp"
#include "ast/ast.hpp"

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
    case expr_t::types::REF: return "REF";
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

expr_t expr_t::deref() const {
  if (type() == types::REF) {
    return get<std::shared_ptr<Ref>>()->get_value().deref();
  }
  return *this;
}

expr_t VariableRef::get_value() const {
  auto it = AST::var_map.find(var_name);
  if (it == AST::var_map.end() || it->second.empty()) {
    throw std::runtime_error("参照先の実体 " + var_name + " が存在しません");
  }
  return it->second.back();
}

void VariableRef::set_value(const expr_t& val) {
  auto it = AST::var_map.find(var_name);
  if (it == AST::var_map.end() || it->second.empty()) {
    throw std::runtime_error("参照先の実体 " + var_name + " が存在しません");
  }
  if (it->second.back().is<expr_t::types::REF>()) {
    it->second.back().get<std::shared_ptr<Ref>>()->set_value(val);
  } else {
    it->second.back() = val;
  }
}

expr_t MatrixElementRef::get_value() const {
  if (index >= matrix->data.size()) {
    throw std::runtime_error("行列要素の参照インデックスが範囲外です");
  }
  return matrix->data[index];
}

void MatrixElementRef::set_value(const expr_t& val) {
  if (index >= matrix->data.size()) {
    throw std::runtime_error("行列要素の参照インデックスが範囲外です");
  }
  if (matrix->data[index].is<expr_t::types::REF>()) {
    matrix->data[index].get<std::shared_ptr<Ref>>()->set_value(val);
  } else {
    matrix->data[index] = val;
  }
}

std::vector<expr_t> Matrix::get_iterable_elements() const {
  std::vector<expr_t> elements;
  if (is_as_mat) {
    for (size_t i = 0; i < rows; ++i) {
      auto row_mat = std::make_shared<Matrix>();
      row_mat->rows = 1;
      row_mat->cols = cols;
      row_mat->data.assign(data.begin() + i * cols, data.begin() + (i + 1) * cols);
      elements.push_back(expr_t(row_mat));
    }
  } else if (rows > 1 && cols > 1) {
    for (size_t i = 0; i < rows; ++i) {
      auto row_mat = std::make_shared<Matrix>();
      row_mat->rows = 1;
      row_mat->cols = cols;
      row_mat->data.assign(data.begin() + i * cols, data.begin() + (i + 1) * cols);
      elements.push_back(expr_t(row_mat));
    }
  } else if (rows > 1 && cols == 1) {
    for (size_t i = 0; i < rows; ++i) {
      elements.push_back(data[i]);
    }
  } else if (rows == 1 && cols > 0) {
    for (size_t j = 0; j < cols; ++j) {
      elements.push_back(data[j]);
    }
  }
  return elements;
}

expr_t::types expr_t::common_type(const expr_t&lhs,const expr_t&rhs){
  expr_t l = lhs.deref();
  expr_t r = rhs.deref();
  types lt=l.type(),rt=r.type();
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
  throw std::runtime_error("型が一致しないため演算できません: " + type_name(l) + " と " + type_name(r));
}

expr_t::operator bool()const{
  expr_t val = deref();
  switch(val.type()){
    case types::VOID: return false;
    case types::BINT: return std::get<bint>(val.value)!=0;
    case types::BFLOAT: return std::get<bfloat>(val.value)!=0;
    case types::BOOL: return std::get<bool>(val.value);
    case types::COMPLEX: {
      auto c = std::get<bcomplex>(val.value);
      return c.real() != 0 || c.imag() != 0;
    }
    case types::STRING: return !std::get<std::string>(val.value).empty();
    default: throw std::runtime_error("無効な型変換 -> bool");
  }
  return false;
}

expr_t::operator bint()const{
  expr_t val = deref();
  switch(val.type()){
    case types::BINT: return std::get<bint>(val.value);
    case types::BFLOAT: return static_cast<bint>(std::get<bfloat>(val.value));
    case types::BOOL: return std::get<bool>(val.value);
    case types::COMPLEX: return static_cast<bint>(std::get<bcomplex>(val.value).real());
    default: throw std::runtime_error("無効な型変換 -> bint");
  }
  return 0;
}

expr_t::operator bfloat()const{
  expr_t val = deref();
  switch(val.type()){
    case types::BINT: return static_cast<bfloat>(std::get<bint>(val.value));
    case types::BFLOAT: return std::get<bfloat>(val.value);
    case types::BOOL: return std::get<bool>(val.value);
    case types::COMPLEX: return std::get<bcomplex>(val.value).real();
    default: throw std::runtime_error("無効な型変換 -> bfloat");
  }
  return 0;
}

expr_t::operator bcomplex()const{
  expr_t val = deref();
  switch(val.type()){
    case types::BINT: return bcomplex(static_cast<bfloat>(std::get<bint>(val.value)), 0);
    case types::BFLOAT: return bcomplex(std::get<bfloat>(val.value), 0);
    case types::BOOL: return bcomplex(std::get<bool>(val.value) ? 1 : 0, 0);
    case types::COMPLEX: return std::get<bcomplex>(val.value);
    default: throw std::runtime_error("無効な型変換 -> bcomplex");
  }
  return bcomplex(0, 0);
}

expr_t::types expr_t::type()const{
  return static_cast<types>(value.index());
}

size_t expr_t::size()const{
  expr_t val = deref();
  if(val.type()==types::MATRIX) return val.get<std::shared_ptr<Matrix>>()->data.size();
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
    for(size_t j=0; j<n; ++j) mat[i][j] = m->data[i * n + j].deref();
    mat[i][n + i] = bint(1);
  }
  for(size_t i=0; i<n; ++i){
    size_t pivot = i;
    for(size_t j=i+1; j<n; ++j){
      if(mat[j][i].deref() != expr_t(bint(0)) && mat[pivot][i].deref() == expr_t(bint(0))) {
        pivot = j;
      }
    }
    if(mat[pivot][i].deref() == expr_t(bint(0))) {
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
  expr_t l = deref();
  expr_t r = rhs.deref();
  if(l.is<types::MATRIX>() && r.is<types::BINT>()){
    return matrix_pow(l.get<std::shared_ptr<Matrix>>(), r.get<bint>());
  }
  if(l.is<types::COMPLEX>() || r.is<types::COMPLEX>()){
    return std::pow((bcomplex)l, (bcomplex)r);
  }
  if(l.is<types::BINT>()&&r.is<types::BINT>())
    if(bint exp_val=r.get<bint>();exp_val>=0)
      return EXPR_T_INTERNAL::pow(l.get<bint>(),exp_val);
  return mp::pow((bfloat)l,(bfloat)r);
}

bool operator&&(const expr_t&lhs,const expr_t&rhs){
  return (bool)lhs.deref()&&(bool)rhs.deref();
}

bool operator||(const expr_t&lhs,const expr_t&rhs){
  return (bool)lhs.deref()||(bool)rhs.deref();
}

bool operator==(const expr_t&lhs,const expr_t&rhs){
  expr_t l = lhs.deref();
  expr_t r = rhs.deref();
  auto lt = l.type(), rt = r.type();
  if(lt == expr_t::types::MATRIX && rt == expr_t::types::MATRIX){
    auto lm = l.get<std::shared_ptr<Matrix>>();
    auto rm = r.get<std::shared_ptr<Matrix>>();
    if(lm->rows != rm->rows || lm->cols != rm->cols) return false;
    for(size_t i=0; i<lm->data.size(); ++i){
      if(lm->data[i] != rm->data[i]) return false;
    }
    return true;
  }
  if((lt == expr_t::types::MATRIX && is_scalar_type(rt)) || (is_scalar_type(lt) && rt == expr_t::types::MATRIX)){
    throw std::runtime_error("行列とスカラーの比較は定義されません");
  }
  switch(expr_t::common_type(l,r)){
    case expr_t::types::BINT:
      return (bint)l==(bint)r;
    case expr_t::types::BFLOAT:
      return (bfloat)l==(bfloat)r;
    case expr_t::types::BOOL:
      return (bool)l==(bool)r;
    case expr_t::types::COMPLEX:
      return (bcomplex)l==(bcomplex)r;
  }
  return false;
}

bool operator!=(const expr_t&lhs,const expr_t&rhs){
  return !(lhs.deref()==rhs.deref());
}

bool operator<(const expr_t&lhs,const expr_t&rhs){
  expr_t l = lhs.deref();
  expr_t r = rhs.deref();
  switch(expr_t::common_type(l,r)){
    case expr_t::types::BINT: return (bint)l<(bint)r;
    case expr_t::types::BFLOAT: return (bfloat)l<(bfloat)r;
    case expr_t::types::BOOL: return (bool)l<(bool)r;
    case expr_t::types::COMPLEX: throw std::runtime_error("複素数の大小比較は定義されません");
  }
  return false;
}

bool operator<=(const expr_t&lhs,const expr_t&rhs){
  return !(lhs.deref()>rhs.deref());
}

bool operator>(const expr_t&lhs,const expr_t&rhs){
  return rhs.deref()<lhs.deref();
}

bool operator>=(const expr_t&lhs,const expr_t&rhs){
  return !(lhs.deref()<rhs.deref());
}

bool expr_t::operator!()const{ return !(bool)deref(); }

expr_t expr_t::operator-()const{
  expr_t val = deref();
  switch(val.type()){
    case types::BINT: return -(bint)val;
    case types::BFLOAT: return -(bfloat)val;
    case types::BOOL: return bint(-(int)(bool)val);
    case types::COMPLEX: return -(bcomplex)val;
    case types::MATRIX: {
      auto m = val.get<std::shared_ptr<Matrix>>();
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
  expr_t l = lhs.deref();
  expr_t r = rhs.deref();
  auto lt = l.type(), rt = r.type();
  if(lt == expr_t::types::MATRIX && rt == expr_t::types::MATRIX){
    auto lm = l.get<std::shared_ptr<Matrix>>();
    auto rm = r.get<std::shared_ptr<Matrix>>();
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
    auto lm = l.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = lm->rows;
    ret->cols = lm->cols;
    ret->data.resize(lm->data.size());
    for(size_t i=0; i<lm->data.size(); ++i)
      ret->data[i] = lm->data[i] + r;
    return ret;
  }
  if(is_scalar_type(lt) && rt == expr_t::types::MATRIX){
    auto rm = r.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = rm->rows;
    ret->cols = rm->cols;
    ret->data.resize(rm->data.size());
    for(size_t i=0; i<rm->data.size(); ++i)
      ret->data[i] = l + rm->data[i];
    return ret;
  }
  switch(expr_t::common_type(l,r)){
    case expr_t::types::BINT: return (bint)l+(bint)r;
    case expr_t::types::BFLOAT: return (bfloat)l+(bfloat)r;
    case expr_t::types::BOOL: return bint((int)(bool)l+(int)(bool)r);
    case expr_t::types::COMPLEX: return (bcomplex)l+(bcomplex)r;
  }
  return expr_t();
}

expr_t operator-(const expr_t&lhs,const expr_t&rhs){
  expr_t l = lhs.deref();
  expr_t r = rhs.deref();
  auto lt = l.type(), rt = r.type();
  if(lt == expr_t::types::MATRIX && rt == expr_t::types::MATRIX){
    auto lm = l.get<std::shared_ptr<Matrix>>();
    auto rm = r.get<std::shared_ptr<Matrix>>();
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
    auto lm = l.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = lm->rows;
    ret->cols = lm->cols;
    ret->data.resize(lm->data.size());
    for(size_t i=0; i<lm->data.size(); ++i)
      ret->data[i] = lm->data[i] - r;
    return ret;
  }
  if(is_scalar_type(lt) && rt == expr_t::types::MATRIX){
    auto rm = r.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = rm->rows;
    ret->cols = rm->cols;
    ret->data.resize(rm->data.size());
    for(size_t i=0; i<rm->data.size(); ++i)
      ret->data[i] = l - rm->data[i];
    return ret;
  }
  switch(expr_t::common_type(l,r)){
    case expr_t::types::BINT: return (bint)l-(bint)r;
    case expr_t::types::BFLOAT: return (bfloat)l-(bfloat)r;
    case expr_t::types::BOOL: return bint((int)(bool)l-(int)(bool)r);
    case expr_t::types::COMPLEX: return (bcomplex)l-(bcomplex)r;
  }
  return expr_t();
}

expr_t operator*(const expr_t&lhs,const expr_t&rhs){
  expr_t l = lhs.deref();
  expr_t r = rhs.deref();
  auto lt = l.type(), rt = r.type();
  if(lt == expr_t::types::MATRIX && rt == expr_t::types::MATRIX){
    auto lm = l.get<std::shared_ptr<Matrix>>();
    auto rm = r.get<std::shared_ptr<Matrix>>();
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
    auto lm = l.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = lm->rows;
    ret->cols = lm->cols;
    ret->data.resize(lm->data.size());
    for(size_t i=0; i<lm->data.size(); ++i)
      ret->data[i] = lm->data[i] * r;
    return ret;
  }
  if(is_scalar_type(lt) && rt == expr_t::types::MATRIX){
    auto rm = r.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = rm->rows;
    ret->cols = rm->cols;
    ret->data.resize(rm->data.size());
    for(size_t i=0; i<rm->data.size(); ++i)
      ret->data[i] = l * rm->data[i];
    return ret;
  }
  switch(expr_t::common_type(l,r)){
    case expr_t::types::BINT: return (bint)l*(bint)r;
    case expr_t::types::BFLOAT: return (bfloat)l*(bfloat)r;
    case expr_t::types::BOOL: return (bool)l&&(bool)r;
    case expr_t::types::COMPLEX: return (bcomplex)l*(bcomplex)r;
  }
  return expr_t();
}

expr_t operator/(const expr_t&lhs,const expr_t&rhs){
  expr_t l = lhs.deref();
  expr_t r = rhs.deref();
  auto lt = l.type(), rt = r.type();
  if(lt == expr_t::types::MATRIX && rt == expr_t::types::MATRIX){
    auto lm = l.get<std::shared_ptr<Matrix>>();
    auto rm = r.get<std::shared_ptr<Matrix>>();
    auto rm_inv = invert_matrix(rm);
    return l * expr_t(rm_inv);
  }
  if(lt == expr_t::types::MATRIX && is_scalar_type(rt)){
    if(r == expr_t(bint(0))) {
      throw std::runtime_error("ゼロ除算エラー");
    }
    auto lm = l.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = lm->rows;
    ret->cols = lm->cols;
    ret->data.resize(lm->data.size());
    for(size_t i=0; i<lm->data.size(); ++i)
      ret->data[i] = lm->data[i] / r;
    return ret;
  }
  if(is_scalar_type(lt) && rt == expr_t::types::MATRIX){
    auto rm = r.get<std::shared_ptr<Matrix>>();
    auto rm_inv = invert_matrix(rm);
    return l * expr_t(rm_inv);
  }
  switch(expr_t::common_type(l,r)){
    case expr_t::types::BINT:{
      bint q,rem;
      mp::divide_qr((bint)l,(bint)r,q,rem);
      if(rem==0) return q;
      return (bfloat)q+(bfloat)rem/(bfloat)r;
    }
    case expr_t::types::BFLOAT: return (bfloat)l/(bfloat)r;
    case expr_t::types::BOOL: return bint((int)(bool)l/(int)(bool)r);
    case expr_t::types::COMPLEX: return (bcomplex)l/(bcomplex)r;
  }
  return expr_t();
}

expr_t operator%(const expr_t&lhs,const expr_t&rhs){
  expr_t l = lhs.deref();
  expr_t r = rhs.deref();
  auto lt = l.type(), rt = r.type();
  if(lt == expr_t::types::MATRIX && is_scalar_type(rt)){
    if(r == expr_t(bint(0))) {
      throw std::runtime_error("剰余演算のゼロ除算エラー");
    }
    auto lm = l.get<std::shared_ptr<Matrix>>();
    auto ret = std::make_shared<Matrix>();
    ret->rows = lm->rows;
    ret->cols = lm->cols;
    ret->data.resize(lm->data.size());
    for(size_t i=0; i<lm->data.size(); ++i)
      ret->data[i] = lm->data[i] % r;
    return ret;
  }
  if(is_scalar_type(lt) && rt == expr_t::types::MATRIX){
    throw std::runtime_error("スカラーに対する行列の剰余算は定義されません");
  }
  switch(expr_t::common_type(l,r)){
    case expr_t::types::BINT: return (bint)l%(bint)r;
    case expr_t::types::BFLOAT: throw std::runtime_error("浮動小数点型の剰余演算は禁止");
    case expr_t::types::BOOL: return bint((int)(bool)l%(int)(bool)r);
    case expr_t::types::COMPLEX: throw std::runtime_error("複素数型の剰余演算は禁止");
  }
  return expr_t();
}

expr_t idiv(const expr_t&lhs,const expr_t&rhs){
  expr_t l = lhs.deref();
  expr_t r = rhs.deref();
  switch(expr_t::common_type(l,r)){
    case expr_t::types::BINT: return (bint)l/(bint)r;
    case expr_t::types::BFLOAT: return mp::trunc((bfloat)l/(bfloat)r);
    case expr_t::types::BOOL: return bint((int)(bool)l/(int)(bool)r);
    case expr_t::types::COMPLEX: throw std::runtime_error("複素数型の割り切り除算は禁止");
  }
  return expr_t();
}

LambdaFunc::~LambdaFunc() {
  delete body;
}