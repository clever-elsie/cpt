#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <filesystem>
#include <vector>
#include "input/input.hpp"

namespace INPUT{

int print_precision = -1;

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
  if(argc==1) // 入力は標準入力から
    return get_source_from_stream(std::cin);
  
  bool has_f = false;
  std::vector<std::string> file_paths;
  for(int i=1; i<argc; ++i){
    if(argv[i]==std::string_view("-f")){
      has_f = true;
      if(i+1 < argc){
        file_paths.push_back(argv[i+1]);
        ++i;
      } else {
        throw std::runtime_error("エラー: -f オプションにはファイルパスが必要です。");
      }
    } else if(argv[i]==std::string_view("-p") || argv[i]==std::string_view("--precision")){
      if(i+1 < argc){
        print_precision = std::stoi(argv[i+1]);
        ++i;
      } else {
        throw std::runtime_error("エラー: -p / --precision オプションには整数値が必要です。");
      }
    } else {
      throw std::runtime_error("エラー: 無効な引数 '" + std::string(argv[i]) + "' が指定されました。\n"
                               "標準入力を使用するか、-f オプションでファイルパスを指定してください。\n"
                               "例: echo '\\i + \\i' | cpt  または  cpt -f file.cpt [-p 桁数]");
    }
  }

  if(has_f){
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

  return get_source_from_stream(std::cin);
}
} // namespace INPUT