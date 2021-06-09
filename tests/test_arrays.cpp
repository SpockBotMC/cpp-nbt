#include <fstream>
#include <sstream>

#undef NDEBUG
#include <cassert>

#include "nbt.hpp"

int main() {

  nbt::TagByteArray test_byte_array {0, 1, 2, 3};
  nbt::TagIntArray test_int_array {0, 1 << 16, 2 << 16, 3 << 16};
  nbt::TagLongArray test_long_array {0, 1LL << 32, 2LL << 32, 3LL << 32};

  nbt::NBT root {"Array Test",
      {
          {"byte array", test_byte_array},
          {"int array", test_int_array},
          {"long array", test_long_array},
      }};


  std::stringstream good_buffer;
  good_buffer << std::ifstream {"arrays.nbt"}.rdbuf();

  std::stringstream test_buffer;
  root.encode(test_buffer);

  assert(!good_buffer.str().compare(test_buffer.str()));

  std::stringstream print_buffer;
  print_buffer << root;
  const std::string printed {print_buffer.str()};

  assert(("printed_byte_array",
      printed.find("byte array: {0, 1, 2, 3}") != printed.npos));

  assert(("printed_int_array",
      printed.find("int array: {0, 65536, 131072, 196608}") != printed.npos));

  assert(("printed_long_array",
      printed.find("long array: {0, 4294967296, 8589934592, 12884901888}") !=
          printed.npos));
}
