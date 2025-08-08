#pragma once
#include "../ast.hpp"
#include "../../tokenizer/tokenize.hpp"

namespace AST{
expr_t sum(std::vector<Nitem*>&args);
expr_t prod(std::vector<Nitem*>&args);
expr_t log(std::vector<Nitem*>&args);
expr_t log10(std::vector<Nitem*>&args);
expr_t abs(std::vector<Nitem*>&args);
expr_t cos(std::vector<Nitem*>&args);
expr_t sin(std::vector<Nitem*>&args);
expr_t tan(std::vector<Nitem*>&args);
expr_t acos(std::vector<Nitem*>&args);
expr_t asin(std::vector<Nitem*>&args);
expr_t atan(std::vector<Nitem*>&args);
expr_t cosh(std::vector<Nitem*>&args);
expr_t sinh(std::vector<Nitem*>&args);
expr_t tanh(std::vector<Nitem*>&args);
expr_t print(std::vector<Nitem*>&args);
expr_t ceil(std::vector<Nitem*>&args);
expr_t floor(std::vector<Nitem*>&args);
expr_t round(std::vector<Nitem*>&args);
expr_t trunc(std::vector<Nitem*>&args);
}// namespace EXPR