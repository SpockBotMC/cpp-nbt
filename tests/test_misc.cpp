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


  // NBT default constructor and r-value empty encode
  nbt::NBT {}.encode(std::ostringstream {});

  // NBT name constructor and named empty encode
  nbt::NBT {"NBT"}.encode(std::ostringstream {});

  // NBT r-value reference constructor and invalid tag exception
  try {
    nbt::NBT {std::istringstream {"not a tag"}};
  } catch(std::exception&) {
  }

  // NBT TagCompound constructor
  nbt::NBT {tag_compound};
}
