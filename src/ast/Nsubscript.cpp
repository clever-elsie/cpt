#include "ast/ast.hpp"
#include <memory>
#include <stdexcept>
#include <string>

namespace AST {

static std::pair<size_t, size_t> resolve_range(const Range& r, size_t len) {
  bint s = r.start;
  bint e = r.end;
  if (s < 0 || e < 0) {
    throw std::runtime_error("範囲インデックスに負数は指定できません");
  }
  size_t start_idx = static_cast<size_t>(s);
  size_t end_idx = static_cast<size_t>(e);
  if (r.is_inclusive) {
    if (start_idx > end_idx || end_idx >= len) {
      throw std::runtime_error("範囲インデックスが範囲外です");
    }
    return { start_idx, end_idx + 1 };
  } else {
    if (start_idx > end_idx || end_idx > len) {
      throw std::runtime_error("範囲インデックスが範囲外です");
    }
    return { start_idx, end_idx };
  }
}

static std::pair<size_t, size_t> resolve_slice_arg(const expr_t& arg, size_t len) {
  if (arg.is<expr_t::types::RANGE>()) {
    return resolve_range(*arg.get<std::shared_ptr<Range>>(), len);
  } else if (arg.is<expr_t::types::BINT>()) {
    bint idx = arg.get<bint>();
    if (idx < 0 || idx >= static_cast<bint>(len)) {
      throw std::runtime_error("インデックスが範囲外です");
    }
    return { static_cast<size_t>(idx), static_cast<size_t>(idx) + 1 };
  } else {
    throw std::runtime_error("スライス指定は範囲または整数である必要があります");
  }
}

Nsubscript::Nsubscript(size_t row, size_t col, Nitem* var_expr, Nitem* index1, Nitem* index2)
  : Nitem(row, col), var_expr(var_expr), index1(index1), index2(index2) {}

Nsubscript::~Nsubscript() {
  delete var_expr;
  delete index1;
  delete index2;
}

expr_t Nsubscript::get_value() {
  expr_t ref = get_reference();
  if (ref.is<expr_t::types::REF>()) {
    return ref.deref();
  }
  return ref;
}

expr_t Nsubscript::get_reference() {
  expr_t base_val = var_expr->get_value();
  expr_t idx1_val = index1->get_value().deref();

  // If index2 is present, evaluate it.
  expr_t idx2_val;
  bool has_idx2 = false;
  if (index2 != nullptr) {
    idx2_val = index2->get_value().deref();
    has_idx2 = true;
  }

  // Handle matrix
  if (base_val.is<expr_t::types::MATRIX>()) {
    auto m = base_val.get<std::shared_ptr<Matrix>>();
    if (m->is_as_mat) {
      // is_as_mat is true: ALWAYS treat as general matrix (R x C)
      if (!has_idx2) {
        // Single index: returns a row reference matrix of shape 1 x C
        std::pair<size_t, size_t> rows_slice = resolve_slice_arg(idx1_val, m->rows);
        size_t slice_len = rows_slice.second - rows_slice.first;
        
        auto row_mat = std::make_shared<Matrix>();
        row_mat->rows = slice_len;
        row_mat->cols = m->cols;
        row_mat->data.resize(slice_len * m->cols);
        for (size_t r = 0; r < slice_len; ++r) {
          size_t orig_r = rows_slice.first + r;
          for (size_t c = 0; c < m->cols; ++c) {
            auto ref = std::make_shared<MatrixElementRef>();
            ref->matrix = m;
            ref->index = orig_r * m->cols + c;
            row_mat->data[r * m->cols + c] = expr_t(ref);
          }
        }
        return expr_t(row_mat);
      } else {
        // Double index: returns scalar reference or submatrix reference
        std::pair<size_t, size_t> rows_slice = resolve_slice_arg(idx1_val, m->rows);
        std::pair<size_t, size_t> cols_slice = resolve_slice_arg(idx2_val, m->cols);
        
        if (idx1_val.is<expr_t::types::BINT>() && idx2_val.is<expr_t::types::BINT>()) {
          // Both are scalars: returns scalar reference
          auto ref = std::make_shared<MatrixElementRef>();
          ref->matrix = m;
          ref->index = rows_slice.first * m->cols + cols_slice.first;
          return expr_t(ref);
        } else {
          // At least one is range: returns reference matrix
          size_t r_len = rows_slice.second - rows_slice.first;
          size_t c_len = cols_slice.second - cols_slice.first;
          auto slice_mat = std::make_shared<Matrix>();
          slice_mat->rows = r_len;
          slice_mat->cols = c_len;
          slice_mat->data.resize(r_len * c_len);
          for (size_t r = 0; r < r_len; ++r) {
            size_t orig_r = rows_slice.first + r;
            for (size_t c = 0; c < c_len; ++c) {
              size_t orig_c = cols_slice.first + c;
              auto ref = std::make_shared<MatrixElementRef>();
              ref->matrix = m;
              ref->index = orig_r * m->cols + orig_c;
              slice_mat->data[r * c_len + c] = expr_t(ref);
            }
          }
          return expr_t(slice_mat);
        }
      }
    } else if (m->rows > 1 && m->cols > 1) {
      // General matrix
      if (!has_idx2) {
        std::pair<size_t, size_t> rows_slice = resolve_slice_arg(idx1_val, m->rows);
        size_t slice_len = rows_slice.second - rows_slice.first;
        
        auto row_mat = std::make_shared<Matrix>();
        row_mat->rows = slice_len;
        row_mat->cols = m->cols;
        row_mat->data.resize(slice_len * m->cols);
        for (size_t r = 0; r < slice_len; ++r) {
          size_t orig_r = rows_slice.first + r;
          for (size_t c = 0; c < m->cols; ++c) {
            auto ref = std::make_shared<MatrixElementRef>();
            ref->matrix = m;
            ref->index = orig_r * m->cols + c;
            row_mat->data[r * m->cols + c] = expr_t(ref);
          }
        }
        return expr_t(row_mat);
      } else {
        std::pair<size_t, size_t> rows_slice = resolve_slice_arg(idx1_val, m->rows);
        std::pair<size_t, size_t> cols_slice = resolve_slice_arg(idx2_val, m->cols);
        
        if (idx1_val.is<expr_t::types::BINT>() && idx2_val.is<expr_t::types::BINT>()) {
          auto ref = std::make_shared<MatrixElementRef>();
          ref->matrix = m;
          ref->index = rows_slice.first * m->cols + cols_slice.first;
          return expr_t(ref);
        } else {
          size_t r_len = rows_slice.second - rows_slice.first;
          size_t c_len = cols_slice.second - cols_slice.first;
          auto slice_mat = std::make_shared<Matrix>();
          slice_mat->rows = r_len;
          slice_mat->cols = c_len;
          slice_mat->data.resize(r_len * c_len);
          for (size_t r = 0; r < r_len; ++r) {
            size_t orig_r = rows_slice.first + r;
            for (size_t c = 0; c < c_len; ++c) {
              size_t orig_c = cols_slice.first + c;
              auto ref = std::make_shared<MatrixElementRef>();
              ref->matrix = m;
              ref->index = orig_r * m->cols + orig_c;
              slice_mat->data[r * c_len + c] = expr_t(ref);
            }
          }
          return expr_t(slice_mat);
        }
      }
    } else if (m->rows > 1 && m->cols == 1) {
      // Column vector (R x 1)
      if (!has_idx2) {
        std::pair<size_t, size_t> rows_slice = resolve_slice_arg(idx1_val, m->rows);
        if (idx1_val.is<expr_t::types::BINT>()) {
          auto ref = std::make_shared<MatrixElementRef>();
          ref->matrix = m;
          ref->index = rows_slice.first;
          return expr_t(ref);
        } else {
          size_t slice_len = rows_slice.second - rows_slice.first;
          auto slice_mat = std::make_shared<Matrix>();
          slice_mat->rows = slice_len;
          slice_mat->cols = 1;
          slice_mat->data.resize(slice_len);
          for (size_t r = 0; r < slice_len; ++r) {
            auto ref = std::make_shared<MatrixElementRef>();
            ref->matrix = m;
            ref->index = rows_slice.first + r;
            slice_mat->data[r] = expr_t(ref);
          }
          return expr_t(slice_mat);
        }
      } else {
        // Double index: column index must be 0
        std::pair<size_t, size_t> rows_slice = resolve_slice_arg(idx1_val, m->rows);
        std::pair<size_t, size_t> cols_slice = resolve_slice_arg(idx2_val, 1);
        if (cols_slice.first != 0) {
          throw std::runtime_error("列ベクトルの列インデックスは0のみ許容されます");
        }
        if (idx1_val.is<expr_t::types::BINT>()) {
          auto ref = std::make_shared<MatrixElementRef>();
          ref->matrix = m;
          ref->index = rows_slice.first;
          return expr_t(ref);
        } else {
          size_t slice_len = rows_slice.second - rows_slice.first;
          auto slice_mat = std::make_shared<Matrix>();
          slice_mat->rows = slice_len;
          slice_mat->cols = 1;
          slice_mat->data.resize(slice_len);
          for (size_t r = 0; r < slice_len; ++r) {
            auto ref = std::make_shared<MatrixElementRef>();
            ref->matrix = m;
            ref->index = rows_slice.first + r;
            slice_mat->data[r] = expr_t(ref);
          }
          return expr_t(slice_mat);
        }
      }
    } else if (m->rows == 1 && m->cols > 1) {
      // Row vector (1 x C)
      if (!has_idx2) {
        std::pair<size_t, size_t> cols_slice = resolve_slice_arg(idx1_val, m->cols);
        if (idx1_val.is<expr_t::types::BINT>()) {
          auto ref = std::make_shared<MatrixElementRef>();
          ref->matrix = m;
          ref->index = cols_slice.first;
          return expr_t(ref);
        } else {
          size_t slice_len = cols_slice.second - cols_slice.first;
          auto slice_mat = std::make_shared<Matrix>();
          slice_mat->rows = 1;
          slice_mat->cols = slice_len;
          slice_mat->data.resize(slice_len);
          for (size_t c = 0; c < slice_len; ++c) {
            auto ref = std::make_shared<MatrixElementRef>();
            ref->matrix = m;
            ref->index = cols_slice.first + c;
            slice_mat->data[c] = expr_t(ref);
          }
          return expr_t(slice_mat);
        }
      } else {
        // Double index: row index must be 0
        std::pair<size_t, size_t> rows_slice = resolve_slice_arg(idx1_val, 1);
        std::pair<size_t, size_t> cols_slice = resolve_slice_arg(idx2_val, m->cols);
        if (rows_slice.first != 0) {
          throw std::runtime_error("行ベクトルの行インデックスは0のみ許容されます");
        }
        if (idx2_val.is<expr_t::types::BINT>()) {
          auto ref = std::make_shared<MatrixElementRef>();
          ref->matrix = m;
          ref->index = cols_slice.first;
          return expr_t(ref);
        } else {
          size_t slice_len = cols_slice.second - cols_slice.first;
          auto slice_mat = std::make_shared<Matrix>();
          slice_mat->rows = 1;
          slice_mat->cols = slice_len;
          slice_mat->data.resize(slice_len);
          for (size_t c = 0; c < slice_len; ++c) {
            auto ref = std::make_shared<MatrixElementRef>();
            ref->matrix = m;
            ref->index = cols_slice.first + c;
            slice_mat->data[c] = expr_t(ref);
          }
          return expr_t(slice_mat);
        }
      }
    } else if (m->rows == 1 && m->cols == 1) {
      // 1x1 matrix
      std::pair<size_t, size_t> rows_slice = resolve_slice_arg(idx1_val, 1);
      if (rows_slice.first != 0) {
        throw std::runtime_error("インデックスが範囲外です");
      }
      if (has_idx2) {
        std::pair<size_t, size_t> cols_slice = resolve_slice_arg(idx2_val, 1);
        if (cols_slice.first != 0) {
          throw std::runtime_error("インデックスが範囲外です");
        }
      }
      auto ref = std::make_shared<MatrixElementRef>();
      ref->matrix = m;
      ref->index = 0;
      return expr_t(ref);
    } else {
      throw std::runtime_error("空の行列に対するアクセスは出来ません");
    }
  }

  // Handle scalar or other types
  if (!has_idx2) {
    if (idx1_val.is<expr_t::types::BINT>() && idx1_val.get<bint>() == 0) {
      return base_val;
    }
    throw std::runtime_error("非行列型オブジェクトに対する0以外のインデックスアクセスは出来ません");
  } else {
    if (idx1_val.is<expr_t::types::BINT>() && idx1_val.get<bint>() == 0 &&
        idx2_val.is<expr_t::types::BINT>() && idx2_val.get<bint>() == 0) {
      return base_val;
    }
    throw std::runtime_error("非行列型オブジェクトに対する0以外のインデックスアクセスは出来ません");
  }
}

json::value Nsubscript::to_json() const {
  json::value v;
  v["type"] = "subscript";
  v["var"] = var_expr ? var_expr->to_json() : json::value();
  v["index1"] = index1 ? index1->to_json() : json::value();
  if (index2) v["index2"] = index2->to_json();
  v["row"] = (int64_t)row;
  v["col"] = (int64_t)col;
  return v;
}

} // namespace AST
