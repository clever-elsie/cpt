#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <filesystem>
#include <vector>
#include "input/input.hpp"

namespace INPUT{

template<class CharT, class Traits>
std::string get_source_from_stream(std::basic_istream<CharT,Traits>&istr){
  return std::string(std::istreambuf_iterator<CharT>(istr),std::istreambuf_iterator<CharT>());
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
      }
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

  throw std::runtime_error("エラー: コマンドライン引数による数式の直接実行は廃止されました。\n"
                           "標準入力を使用するか、-f オプションでファイルパスを指定してください。\n"
                           "例: echo '\\i + \\i' | cpt  または  cpt -f file.cpt");
}
} // namespace INPUT