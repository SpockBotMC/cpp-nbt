#include <fstream>
#include <sstream>

#undef NDEBUG
#include <cassert>

#include "nbt.hpp"

int main() {

  // TagCompound store/get
  nbt::TagCompound tag_compound;
  tag_compound["Hello"] = "World";
  const std::string key {"Hello"};
  assert(std::get<nbt::TagString>(tag_compound[key]) ==
      std::get<nbt::TagString>(tag_compound.at(key)));

  std::stringstream print_compound;
  print_compound << tag_compound;
}
