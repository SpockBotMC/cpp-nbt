#include <fstream>
#include <sstream>

#undef NDEBUG
#include <cassert>

#include "nbt.hpp"

int main() {

  nbt::TagCompound test_compound {};

  nbt::NBT root {"Compound Test", {{"compound", test_compound}}};


  std::stringstream good_buffer;
  good_buffer << std::ifstream {"compound.nbt"}.rdbuf();

  std::stringstream test_buffer;
  root.encode(test_buffer);

  assert(("binary_compound", good_buffer.str() == test_buffer.str()));

  root.decode(test_buffer);

  std::stringstream print_buffer;
  print_buffer << root;

  const std::string expected {
      "\"Compound Test\"\n"
      "<TagCompound> {\n"
      "  compound: <TagCompound> {}\n"
      "}"};

  assert(("printed_compound", expected == print_buffer.str()));
}
