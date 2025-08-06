#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <filesystem>
#include "input.hpp"

namespace INPUT{

template<class CharT, class Traits>
std::string get_source_from_stream(std::basic_istream<CharT,Traits>&istr){
  return std::string(std::istreambuf_iterator<CharT>(istr),std::istreambuf_iterator<CharT>());
}

std::string get_all_source_input(int argc, char**argv){
  namespace fs=std::filesystem;
  if(argc==1) // 入力は標準入力から
    return get_source_from_stream(std::cin);
  //ファイルが存在するならそれを入力とする
  if(argc==3){
    if(argv[1]==std::string_view("-f")){
      if(std::ifstream ifile(argv[2]);ifile)
        return get_source_from_stream(ifile);
    }else if(argv[2]==std::string_view("-f"))
      if(std::ifstream ifile(argv[1]);ifile)
        return get_source_from_stream(ifile);
  }
  std::string buffer;
  for(int i=1;i<argc;++i){
    buffer+=argv[i];
    if(i+1!=argc)
      buffer.push_back(' ');
  }
  return buffer;
}
} // namespace INPUT