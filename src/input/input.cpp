#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <filesystem>
#include <stdexcept>
#include <string_view>
#include <vector>
#include "input/input.hpp"

namespace INPUT{

int print_precision = -1;
bool dump_ast = false;
input_mode mode = input_mode::standard_input;
std::string source_name = "<stdin>";

template<class CharT, class Traits>
std::string get_source_from_stream(std::basic_istream<CharT,Traits>&istr){
  std::string result;
  std::string line;
  while(std::getline(istr, line)){
    result += line;
    result += "\n";
    std::string trimmed = line;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
    trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
    if(trimmed == "exit" || trimmed == "exit;"){
      break;
    }
  }
  return result;
}

std::string get_all_source_input(int argc, char**argv){
  bool has_f = false;
  std::vector<std::string> file_paths;
  std::vector<std::string> command_line_source;
  bool end_of_options = false;

  for(int i=1; i<argc; ++i){
    std::string_view arg(argv[i]);

    if(!end_of_options && arg == "--"){
      end_of_options = true;
    } else if(!end_of_options && arg == "-f"){
      has_f = true;
      if(i+1 < argc){
        file_paths.push_back(argv[i+1]);
        ++i;
      } else {
        throw std::runtime_error("エラー: -f オプションにはファイルパスが必要です。");
      }
    } else if(!end_of_options && (arg == "-p" || arg == "--precision")){
      if(i+1 < argc){
        try{
          std::string precision_arg(argv[i+1]);
          size_t parsed = 0;
          print_precision = std::stoi(precision_arg, &parsed);
          if(parsed != precision_arg.size()) throw std::invalid_argument("precision");
        }catch(const std::exception&){
          throw std::runtime_error("エラー: -p / --precision オプションには整数値が必要です。");
        }
        ++i;
      } else {
        throw std::runtime_error("エラー: -p / --precision オプションには整数値が必要です。");
      }
    } else if(!end_of_options && arg == "--ast"){
      dump_ast = true;
    } else if(!end_of_options && arg.starts_with("--")){
      throw std::runtime_error("エラー: 無効な引数 '" + std::string(arg) + "' が指定されました。\n"
                               "数式を直接実行するか、標準入力を使用するか、-f オプションでファイルパスを指定してください。\n"
                               "例: cpt '1 + 2' または echo '\\i + \\i' | cpt  または  cpt -f file.cpt [-p 桁数]");
    } else {
      command_line_source.emplace_back(arg);
    }
  }

  if(has_f){
    if(!command_line_source.empty()){
      throw std::runtime_error("エラー: -f オプションとコマンドライン上の数式は併用できません。");
    }

    mode = input_mode::file;
    source_name = file_paths.front();
    std::string combined;
    for(const auto& path : file_paths){
      if(std::ifstream ifile(path); ifile){
        combined += get_source_from_stream(ifile);
        combined += "\n";
      }else{
        throw std::runtime_error("ファイルを開くことができませんでした: " + path);
      }
    }
    return combined;
  }

  if(!command_line_source.empty()){
    mode = input_mode::command_line;
    source_name = "<command line>";
    std::string combined;
    for(size_t i=0; i<command_line_source.size(); ++i){
      if(i != 0) combined.push_back(' ');
      combined += command_line_source[i];
    }
    return combined;
  }

  mode = input_mode::standard_input;
  source_name = "<stdin>";
  return get_source_from_stream(std::cin);
}
} // namespace INPUT
