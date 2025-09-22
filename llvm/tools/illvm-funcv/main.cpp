// #include "illvm/FuncV/ELF/FuncV.h"
// #include "illvm/Support/FileSystem.h"

#include <optional>
#include <iostream>

class A {
public:
  int x = 12;
  ~A() {
    std::cout << "des A" << std::endl;
  }
};

int main(const int argc, char **argv) {
  // if (argc < 5) {
  //   llvm::errs() << "Usage: " << argv[0]
  //                << "<old-obj-path> <new-obj-path> <output-path> <funcx-path>\n";
  //   return 1;
  // }
  //
  // const std::string oldObjPath = argv[1];
  // const std::string newObjPath = argv[2];
  // const std::string outputPath = argv[3];
  // const std::string funcXPath = argv[4];
  //
  // // Read funcx set
  // auto lines = illvm::FileSystem::readLines(funcXPath);
  // const std::unordered_set<std::string> funcXSet(lines.begin(), lines.end());
  //
  // illvm::funcv::elf::FuncV::run(oldObjPath, newObjPath, outputPath, funcXSet);

  std::optional<A> opt;
  std::cout << opt.has_value() << std::endl;
  opt.emplace();
  std::cout << opt.has_value() << std::endl;
  if (opt) {
    std::cout << opt.value().x << std::endl;
  }
  return 0;
}