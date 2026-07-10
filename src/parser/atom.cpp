#include "parser/expr.hpp"
#include "parser/top.hpp"
#include <array>
#include <algorithm>
#include <ranges>

namespace EXPR{

std::vector<AST::Nitem*> get_args(tokenize&tok){
  std::vector<AST::Nitem*> args;
  if(tok.top().symbol!=symbol_t::LPAREN) tok.error_throw(__func__+std::string(" : 関数呼び出しの引数が(で囲まれていません"));
  tok.next_token(); // (を消費
  if(tok.top().symbol==symbol_t::RPAREN){
    tok.next_token();
    return args;
  }
  while(true){
    args.push_back(expr(tok));
    if(tok.top().symbol==symbol_t::COMMA) tok.next_token();
    else if(tok.top().symbol==symbol_t::RPAREN) break;
    else tok.error_throw(__func__+std::string(" : 関数呼び出しの引数が,で区切られていません"));
  }
  if(tok.top().symbol!=symbol_t::RPAREN) tok.error_throw(__func__+std::string(" : 関数呼び出しの引数が)で囲まれていません"));
  tok.next_token();
  return args;
}

template<bool is_below_declare>
std::pair<AST::Nitem*,AST::Nitem*> get_right_args(tokenize&tok){ // {上付き，下付き}
  AST::Nitem* exp=nullptr, *below=nullptr;
  auto get_arg=[&tok](AST::Nitem*&arg,bool is_below)->void {
    tok.next_token();
    if(tok.top().symbol==symbol_t::LCURLY){
      if(is_below&&is_below_declare)
        arg=PARSER::define_var(tok,nullptr);
      else{
        tok.next_token();
        arg=expr(tok);
      }
      if(tok.top().symbol!=symbol_t::RCURLY) tok.error_throw(__func__+std::string(" : {}が閉じられていません"));
      tok.next_token();
    }else{
      if(is_below&&is_below_declare) tok.error_throw(__func__+std::string(" : 下付きの引数が宣言されていません"));
      arg=atom(tok);
    }
  };
  while(exp==nullptr||below==nullptr){
    if(tok.top().symbol==symbol_t::CARET) get_arg(exp,false);
    else if(tok.top().symbol==symbol_t::UNDERSCORE) get_arg(below,true);
    else break;
  }
  return{exp,below};
}

AST::Nitem* function_call(tokenize&tok){
  std::string_view name=tok.top().token;
  tok.next_token(); // 関数名を消費
  // 上付きべき乗があるときはpowのノードを作成する
  auto [row,col]=tok.get_pos();
  auto [exp,below]=get_right_args<false>(tok);
  AST::Nfn* ret=new AST::Nfn(row,col,name,get_args(tok));
  if(below!=nullptr) throw std::runtime_error("関数"+std::string(name)+"に下付きの引数はありません");
  if(exp!=nullptr) return new AST::Nexpr(row,col,AST::op_t::POW,ret,exp);
  return ret;
}

AST::Nitem* reserved_function_call(tokenize&tok){
  std::string_view name=tok.top().token;
  // 上付きと下付きが両方あるとき，\sumや\prodでは必ず，上付き，下付きの順番に第一引数，第二引数に入る
  // 下付きだけのlogは第一引数に底を入れる．もし底がないとき，自然対数の底eを底とし，第一引数はnullptrとする．
  if(name=="\\log"){ // 下付き特殊
    tok.next_token();
    auto [row,col]=tok.get_pos();
    auto[exp,base]=get_right_args<false>(tok);
    std::vector<AST::Nitem*> arg=get_args(tok);
    std::vector<AST::Nitem*>args{base};
    args.insert(args.end(),arg.begin(),arg.end());
    AST::Nitem* ret=new AST::Nfn(row,col,name,std::move(args));
    if(exp!=nullptr) return new AST::Nexpr(row,col,AST::op_t::POW,ret,exp);
    return ret;
  }else if(name=="\\sum"||name=="\\prod"){ // 上下付き特殊
    tok.next_token();
    auto [row,col]=tok.get_pos();
    auto[exp,base]=get_right_args<true>(tok);
    std::vector<AST::Nitem*>args{exp,base};
    args.push_back(expr(tok));
    return new AST::Nfn(row,col,name,std::move(args));
  }else return function_call(tok); // 普通の関数
}

template<size_t base>
AST::Nitem* parse_literal(tokenize&tok){
  if constexpr(base==2){
    // 0b{0,1}+ -> HEX
    std::string_view token=tok.top().token;
    auto [row,col]=tok.get_pos();
    tok.next_token();
    size_t rem=(token.size()-2)&0b11;
    size_t len=(rem>0)+((token.size()-2)>>2);
    size_t idx=2;
    std::string hex("0x");
    hex.reserve(len);
    if(char p=0;rem){
      if(rem==3&&token[idx++]=='1') p+=4;
      if(rem>=2&&token[idx++]=='1') p+=2;
      if(token[idx++]=='1') p++;
      hex.push_back(p<=9?'0'+p:'A'+p-10);
    }
    for(;idx<token.size();idx+=4){
      char p=(token[idx]-'0'<<3)+(token[idx+1]-'0'<<2)+(token[idx+2]-'0'<<1)+token[idx+3]-'0';
      hex.push_back(p<=9?'0'+p:'A'+p-10);
    }
    return new AST::Nliteral(row,col,bint(hex));
  }else{
    pToken tmp=tok.top();
    tok.next_token();
    auto [row,col]=tok.get_pos();
    if constexpr(base==0) // float
      return new AST::Nliteral(row,col,bfloat(tmp.token));
    else if constexpr(base==10||base==16)
      return new AST::Nliteral(row,col,bint(tmp.token));
  }
}

AST::Nitem* parse_if(tokenize&tok){
  auto [row, col] = tok.get_pos();
  tok.next_token();
  if(tok.top().symbol != symbol_t::LPAREN) tok.error_exit("ifの条件式は()で囲んでください");
  tok.next_token();
  AST::Nitem* cond = expr(tok);
  if(tok.top().symbol != symbol_t::RPAREN) tok.error_exit("ifの条件式の閉じカッコがありません");
  tok.next_token();
  AST::Nitem* then_expr = nullptr;
  if(tok.top().symbol == symbol_t::LCURLY){
    tok.next_token();
    auto body = new AST::Nstat();
    PARSER::second(tok, false, body);
    if(tok.top().symbol != symbol_t::RCURLY) tok.error_exit("ifブロックが閉じられていません");
    tok.next_token();
    then_expr = body;
  } else {
    then_expr = expr(tok);
  }
  if(tok.top().symbol == symbol_t::SEMICOLON) {
    tokenize lookahead = tok;
    lookahead.next_token();
    if(lookahead.top().token == "else"){
      tok.next_token();
    }
  }
  AST::Nitem* else_expr = nullptr;
  if(tok.top().token == "else"){
    tok.next_token();
    if(tok.top().symbol == symbol_t::LCURLY){
      tok.next_token();
      auto body = new AST::Nstat();
      PARSER::second(tok, false, body);
      if(tok.top().symbol != symbol_t::RCURLY) tok.error_exit("elseブロックが閉じられていません");
      tok.next_token();
      else_expr = body;
    } else {
      else_expr = expr(tok);
    }
  }
  return new AST::Nif(row, col, cond, then_expr, else_expr);
}

AST::Nitem* parse_while(tokenize&tok){
  auto [row, col] = tok.get_pos();
  tok.next_token();
  if(tok.top().symbol != symbol_t::LPAREN) tok.error_exit("whileの条件式は()で囲んでください");
  tok.next_token();
  AST::Nitem* cond = expr(tok);
  if(tok.top().symbol != symbol_t::RPAREN) tok.error_exit("whileの条件式の閉じカッコがありません");
  tok.next_token();
  AST::Nitem* body = nullptr;
  if(tok.top().symbol == symbol_t::LCURLY){
    tok.next_token();
    auto body_stat = new AST::Nstat();
    PARSER::second(tok, false, body_stat);
    if(tok.top().symbol != symbol_t::RCURLY) tok.error_exit("whileブロックが閉じられていません");
    tok.next_token();
    body = body_stat;
  } else {
    body = expr(tok);
  }
  return new AST::Nwhile(row, col, cond, body);
}

AST::Nitem* parse_for(tokenize&tok){
  auto [row, col] = tok.get_pos();
  tok.next_token();
  if(tok.top().type != token_t::IDENT) tok.error_exit("forループ変数名がありません");
  std::string_view var_name = tok.top().token;
  tok.next_token();
  if(tok.top().token != "in") tok.error_exit("forループには 'in' が必要です");
  tok.next_token();
  AST::Nitem* range_expr = expr(tok);
  AST::Nitem* body = nullptr;
  if(tok.top().symbol == symbol_t::LCURLY){
    tok.next_token();
    auto body_stat = new AST::Nstat();
    PARSER::second(tok, false, body_stat);
    if(tok.top().symbol != symbol_t::RCURLY) tok.error_exit("forブロックが閉じられていません");
    tok.next_token();
    body = body_stat;
  } else {
    body = expr(tok);
  }
  return new AST::Nfor(row, col, var_name, range_expr, body);
}

AST::Nitem* parse_lambda(tokenize&tok){
  auto [row, col] = tok.get_pos();
  tok.next_token();
  std::vector<std::string> args;
  while(tok.top().type == token_t::IDENT){
    args.push_back(std::string(tok.top().token));
    tok.next_token();
    if(tok.top().symbol == symbol_t::COMMA){
      tok.next_token();
    } else break;
  }
  if(tok.top().symbol != symbol_t::RPAREN) tok.error_exit("ラムダ式の引数リストの閉じカッコがありません");
  tok.next_token();
  if(tok.top().symbol != symbol_t::LCURLY) tok.error_exit("ラムダ式のボディは {} で囲む必要があります");
  tok.next_token();
  auto body = new AST::Nstat();
  body->args = args;
  for(const auto& a : args) body->args_set.insert(a);
  PARSER::second(tok, true, body);
  if(tok.top().symbol != symbol_t::RCURLY) tok.error_exit("ラムダ式のボディが閉じられていません");
  tok.next_token();
  return new AST::Nlambda(row, col, std::move(args), body);
}

AST::Nitem* parse_list(tokenize& tok){
  auto [row, col] = tok.get_pos();
  tok.next_token();
  if(tok.top().symbol == symbol_t::RSQUARE){
    tok.next_token();
    return new AST::Nvector(row, col, {});
  }
  std::vector<std::vector<AST::Nitem*>> grid;
  grid.push_back({});
  while(true){
    grid.back().push_back(expr(tok));
    auto sym = tok.top().symbol;
    if(sym == symbol_t::COMMA){
      tok.next_token();
    } else if(sym == symbol_t::SEMICOLON){
      tok.next_token();
      if(tok.top().symbol != symbol_t::RSQUARE){
        grid.push_back({});
      }
    } else if(sym == symbol_t::RSQUARE){
      break;
    } else {
      tok.error_exit("リスト要素の区切りがありません");
    }
  }
  tok.next_token();
  size_t rows = grid.size();
  size_t cols = grid[0].size();
  if(rows == 1){
    return new AST::Nvector(row, col, std::move(grid[0]));
  }
  std::vector<AST::Nitem*> flat_elements;
  for(size_t i=0; i<rows; ++i){
    if(grid[i].size() != cols){
      tok.error_exit("行列の各行の列数が一致しません");
    }
    for(auto elem : grid[i]){
      flat_elements.push_back(elem);
    }
  }
  return new AST::Nmatrix(row, col, rows, cols, std::move(flat_elements));
}

AST::Nitem* atom(tokenize&tok) {
  if(token_t::EMPTY==tok.top().type)
    tok.error_throw(__func__+std::string(" : 項が空文字列です"));
  else if(token_t::IDENT==tok.top().type){
    if(tok.top().token == "if") return parse_if(tok);
    if(tok.top().token == "while") return parse_while(tok);
    if(tok.top().token == "for") return parse_for(tok);
    tokenize lookahead = tok;
    lookahead.next_token();
    if(lookahead.top().symbol == symbol_t::CC){
      auto [row, col] = tok.get_pos();
      std::string_view alias = tok.top().token;
      tok.next_token();
      tok.next_token();
      if(tok.top().type != token_t::IDENT) tok.error_exit("名前空間解決の後に識別子がありません");
      std::string_view name = tok.top().token;
      tok.next_token();
      if(tok.top().symbol == symbol_t::LPAREN){
        std::string full_name = std::string(alias) + "::" + std::string(name);
        auto [exp, below] = get_right_args<false>(tok);
        AST::Nfn* ret = new AST::Nfn(row, col, full_name, get_args(tok));
        if(below != nullptr) throw std::runtime_error("関数" + full_name + "に下付きの引数はありません");
        if(exp != nullptr) return new AST::Nexpr(row, col, AST::op_t::POW, ret, exp);
        return ret;
      }
      return new AST::Nns_resolve(row, col, alias, name);
    }
    bool is_fn = false;
    if(AST::fn_map.find(tok.top().token) != AST::fn_map.end()) {
      is_fn = true;
    } else {
      tokenize lookahead = tok;
      lookahead.next_token();
      auto sym = lookahead.top().symbol;
      if(sym == symbol_t::LPAREN || sym == symbol_t::CC || sym == symbol_t::UNDERSCORE) {
        is_fn = true;
      }
    }
    if(is_fn)
      return function_call(tok);
    auto [row,col]=tok.get_pos();
    AST::Nitem* ret=new AST::Nvar(row,col,tok.top().token);
    tok.next_token();
    return ret;
  }else if(token_t::RESERVED==tok.top().type) return reserved_function_call(tok);
  else if(tok.top().symbol==symbol_t::EXCL){
    auto [row,col]=tok.get_pos();
    tok.next_token();
    return new AST::Nexpr(row,col,AST::op_t::NOT,atom(tok),nullptr);
  }else if(tok.top().symbol==symbol_t::MINUS){
    auto [row,col]=tok.get_pos();
    tok.next_token();
    return new AST::Nexpr(row,col,AST::op_t::NEG,atom(tok),nullptr);
  }else if(tok.top().symbol==symbol_t::LSQUARE){
    return parse_list(tok);
  }else if(tok.top().symbol==symbol_t::LCURLY){
    tok.next_token();
    AST::Nitem* ret=expr(tok);
    if(tok.top().symbol!=symbol_t::RCURLY) tok.error_throw("中括弧が閉じられていません");
    tok.next_token();
    return ret;
  }else if(tok.top().symbol==symbol_t::LPAREN){
    bool is_lambda = false;
    {
      tokenize lookahead = tok;
      lookahead.next_token();
      int depth = 1;
      while(lookahead.top().type != token_t::EMPTY){
        auto sym = lookahead.top().symbol;
        if(sym == symbol_t::LPAREN) depth++;
        else if(sym == symbol_t::RPAREN){
          depth--;
          if(depth == 0){
            lookahead.next_token();
            if(lookahead.top().symbol == symbol_t::LCURLY){
              is_lambda = true;
            }
            break;
          }
        }
        lookahead.next_token();
      }
    }
    if(is_lambda){
      return parse_lambda(tok);
    }
    tok.next_token();
    AST::Nitem* ret=expr(tok);
    if(tok.top().symbol!=symbol_t::RPAREN) tok.error_throw(__func__+std::string(" : かっこが閉じられてないよ"));
    tok.next_token();
    return ret;
  }else if(token_t::DECIMAL==tok.top().type) return parse_literal<10>(tok);
  else if(token_t::FLOAT==tok.top().type) return parse_literal<0>(tok);
  else if(token_t::BINARY==tok.top().type) return parse_literal<2>(tok);
  else if(token_t::HEX==tok.top().type) return parse_literal<16>(tok);
  else if(token_t::STRING==tok.top().type){
    auto [row, col] = tok.get_pos();
    std::string_view token = tok.top().token;
    tok.next_token();
    std::string val(token.begin() + 1, token.end() - 1);
    std::string processed;
    for(size_t i=0; i<val.size(); ++i){
      if(val[i] == '\\' && i + 1 < val.size()){
        i++;
        if(val[i] == 'n') processed += '\n';
        else if(val[i] == 't') processed += '\t';
        else processed += val[i];
      } else {
        processed += val[i];
      }
    }
    return new AST::Nliteral(row, col, expr_t(processed));
  }else if(token_t::COMPLEX==tok.top().type){
    auto [row, col] = tok.get_pos();
    std::string_view token = tok.top().token;
    tok.next_token();
    std::string val(token.begin(), token.end() - 1);
    bfloat imag = bfloat(val);
    return new AST::Nliteral(row, col, expr_t(bcomplex(0, imag)));
  }else if(token_t::SYMBOL==tok.top().type)
    tok.error_throw(__func__+std::string(" : ")+std::string(tok.top().token)+"は無効な記号です");
  return nullptr;
}

} //namespace EXPR