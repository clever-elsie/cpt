#include "ast/reserved.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>

namespace AST {

// ヘルパー関数: イテレータの要素を VECTOR から取り出す
static std::vector<expr_t> get_iterable_elements(expr_t val){
  if(val.is<expr_t::types::VECTOR>()){
    return val.get<std::vector<expr_t>>();
  }
  if(val.is<expr_t::types::RANGE>()){
    auto r = val.get<std::shared_ptr<Range>>();
    std::vector<expr_t> ret;
    bint current = r->start;
    while(true){
      bool cond = r->is_inclusive ? (current <= r->end) : (current < r->end);
      if(!cond) break;
      ret.push_back(expr_t(current));
      current += 1;
    }
    return ret;
  }
  throw std::runtime_error("イテレート可能なオブジェクトではありません");
}

// ヘルパー関数: 関数オブジェクト (ラムダ) の呼び出し
static expr_t call_function_object(expr_t fn_val, const std::vector<expr_t>& args){
  if(!fn_val.is<expr_t::types::FUNCTION>())
    throw std::runtime_error("呼び出し対象が関数オブジェクトではありません");
  auto l = fn_val.get<std::shared_ptr<LambdaFunc>>();
  
  for(const auto& [vname, vval] : l->closure_env){
    var_map[vname].push_back(vval);
  }
  expr_t ret;
  try {
    ret = l->body->evaluate(std::vector<expr_t>(args));
  } catch (...) {
    for(const auto& [vname, vval] : l->closure_env){
      var_map[vname].pop_back();
    }
    throw;
  }
  for(const auto& [vname, vval] : l->closure_env){
    var_map[vname].pop_back();
  }
  return ret;
}

expr_t T(std::vector<Nitem*>&args){
  if(args.size() != 1) throw std::runtime_error("T関数には引数が1つ必要です");
  expr_t val = args[0]->get_value();
  if(!val.is<expr_t::types::MATRIX>()) throw std::runtime_error("T関数は行列に対してのみ適用できます");
  auto m = val.get<std::shared_ptr<Matrix>>();
  auto ret = std::make_shared<Matrix>();
  ret->rows = m->cols;
  ret->cols = m->rows;
  ret->data.resize(m->rows * m->cols);
  for(size_t i=0; i<m->rows; ++i){
    for(size_t j=0; j<m->cols; ++j){
      ret->data[j * m->rows + i] = m->data[i * m->cols + j];
    }
  }
  return expr_t(ret);
}

expr_t t(std::vector<Nitem*>&args){
  if(args.size() != 1) throw std::runtime_error("t関数には引数が1つ必要です");
  expr_t val = args[0]->get_value();
  if(!val.is<expr_t::types::MATRIX>()) throw std::runtime_error("t関数は行列に対してのみ適用できます");
  auto m = val.get<std::shared_ptr<Matrix>>();
  auto ret = std::make_shared<Matrix>();
  ret->rows = m->cols;
  ret->cols = m->rows;
  ret->data.resize(m->rows * m->cols);
  for(size_t i=0; i<m->rows; ++i){
    for(size_t j=0; j<m->cols; ++j){
      expr_t elem = m->data[i * m->cols + j];
      if(elem.is<expr_t::types::COMPLEX>()){
        auto c = elem.get<bcomplex>();
        ret->data[j * m->rows + i] = std::conj(c);
      } else {
        ret->data[j * m->rows + i] = elem;
      }
    }
  }
  return expr_t(ret);
}

expr_t dot(std::vector<Nitem*>&args){
  if(args.size() != 2) throw std::runtime_error("dot関数には引数が2つ必要です");
  expr_t lhs = args[0]->get_value();
  expr_t rhs = args[1]->get_value();
  if(lhs.is<expr_t::types::MATRIX>() && rhs.is<expr_t::types::MATRIX>()){
    auto lm = lhs.get<std::shared_ptr<Matrix>>();
    auto rm = rhs.get<std::shared_ptr<Matrix>>();
    if(lm->rows != rm->rows || lm->cols != rm->cols)
      throw std::runtime_error("dot関数の行列サイズが一致しません");
    auto ret = std::make_shared<Matrix>();
    ret->rows = lm->rows;
    ret->cols = lm->cols;
    ret->data.resize(lm->data.size());
    for(size_t i=0; i<lm->data.size(); ++i){
      ret->data[i] = lm->data[i] * rm->data[i];
    }
    return expr_t(ret);
  }
  if(lhs.is<expr_t::types::VECTOR>() && rhs.is<expr_t::types::VECTOR>()){
    const auto& lv = lhs.get<std::vector<expr_t>>();
    const auto& rv = rhs.get<std::vector<expr_t>>();
    if(lv.size() != rv.size()) throw std::runtime_error("dot関数のベクトルサイズが一致しません");
    std::vector<expr_t> ret(lv.size());
    for(size_t i=0; i<lv.size(); ++i){
      ret[i] = lv[i] * rv[i];
    }
    return expr_t(ret);
  }
  throw std::runtime_error("dot関数は行列またはベクトルに対してのみ定義されます");
}

expr_t read_impl() {
  std::string line;
  bool has_line = false;
  while(!AST::input_buffer.empty()){
    size_t pos = AST::input_buffer.find('\n');
    std::string candidate;
    if(pos != std::string::npos){
      candidate = AST::input_buffer.substr(0, pos);
      AST::input_buffer = AST::input_buffer.substr(pos + 1);
    } else {
      candidate = AST::input_buffer;
      AST::input_buffer.clear();
    }
    size_t first = candidate.find_first_not_of(" \t\r\n");
    if(first != std::string::npos){
      size_t last = candidate.find_last_not_of(" \t\r\n");
      line = candidate.substr(first, (last - first + 1));
      has_line = true;
      break;
    }
  }
  if(!has_line){
    while(std::getline(std::cin, line)){
      size_t first = line.find_first_not_of(" \t\r\n");
      if(first != std::string::npos){
        size_t last = line.find_last_not_of(" \t\r\n");
        line = line.substr(first, (last - first + 1));
        has_line = true;
        break;
      }
    }
  }
  if(has_line){
    if(line.find('.') != std::string::npos || line.find('e') != std::string::npos || line.find('E') != std::string::npos){
      try {
        return expr_t(bfloat(line));
      } catch(...) {}
    }
    try {
      return expr_t(bint(line));
    } catch(...) {}
    return expr_t(line);
  }
  return expr_t(std::monostate{});
}

expr_t read(std::vector<Nitem*>&args){
  if(args.size() != 0) throw std::runtime_error("read関数には引数は不要です");
  return read_impl();
}

expr_t input(std::vector<Nitem*>&args){
  if(args.size() != 0) throw std::runtime_error("input関数には引数は不要です");
  return read_impl();
}

expr_t take(std::vector<Nitem*>&args){
  if(args.size() != 2) throw std::runtime_error("take関数には引数が2つ必要です");
  expr_t iterable = args[0]->get_value();
  expr_t k_val = args[1]->get_value();
  if(!k_val.is<expr_t::types::BINT>()) throw std::runtime_error("takeの個数は整数でなければなりません");
  bint k = k_val.get<bint>();
  auto elements = get_iterable_elements(iterable);
  std::vector<expr_t> ret;
  for(bint i=0; i<k && i<elements.size(); ++i){
    ret.push_back(elements[static_cast<size_t>(i)]);
  }
  return expr_t(ret);
}

expr_t drop(std::vector<Nitem*>&args){
  if(args.size() != 2) throw std::runtime_error("drop関数には引数が2つ必要です");
  expr_t iterable = args[0]->get_value();
  expr_t n_val = args[1]->get_value();
  if(!n_val.is<expr_t::types::BINT>()) throw std::runtime_error("dropの個数は整数でなければなりません");
  bint n = n_val.get<bint>();
  auto elements = get_iterable_elements(iterable);
  std::vector<expr_t> ret;
  for(size_t i=static_cast<size_t>(n); i<elements.size(); ++i){
    ret.push_back(elements[i]);
  }
  return expr_t(ret);
}

expr_t filter(std::vector<Nitem*>&args){
  if(args.size() != 2) throw std::runtime_error("filter関数には引数が2つ必要です");
  expr_t iterable = args[0]->get_value();
  expr_t pred = args[1]->get_value();
  auto elements = get_iterable_elements(iterable);
  std::vector<expr_t> ret;
  for(auto& elem : elements){
    expr_t cond = call_function_object(pred, {elem});
    if((bool)cond){
      ret.push_back(elem);
    }
  }
  return expr_t(ret);
}

expr_t transform(std::vector<Nitem*>&args){
  if(args.size() != 2) throw std::runtime_error("transform関数には引数が2つ必要です");
  expr_t iterable = args[0]->get_value();
  expr_t pred = args[1]->get_value();
  auto elements = get_iterable_elements(iterable);
  std::vector<expr_t> ret;
  for(auto& elem : elements){
    ret.push_back(call_function_object(pred, {elem}));
  }
  return expr_t(ret);
}

expr_t enumerate(std::vector<Nitem*>&args){
  if(args.size() != 1) throw std::runtime_error("enumerate関数には引数が1つ必要です");
  expr_t iterable = args[0]->get_value();
  auto elements = get_iterable_elements(iterable);
  std::vector<expr_t> ret;
  for(size_t i=0; i<elements.size(); ++i){
    std::vector<expr_t> pair;
    pair.push_back(expr_t(bint(i)));
    pair.push_back(elements[i]);
    ret.push_back(expr_t(pair));
  }
  return expr_t(ret);
}

expr_t reverse(std::vector<Nitem*>&args){
  if(args.size() != 1) throw std::runtime_error("reverse関数には引数が1つ必要です");
  expr_t iterable = args[0]->get_value();
  auto elements = get_iterable_elements(iterable);
  std::reverse(elements.begin(), elements.end());
  return expr_t(elements);
}

} // namespace AST
