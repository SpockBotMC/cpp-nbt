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

  assert(("binary_arrays", good_buffer.str() == test_buffer.str()));


  nbt::NBT file {good_buffer};

  assert(("test_byte_arry",
      root.data->tags.at("byte array") == file.data->tags.at("byte array")));

  assert(("test_int_array",
      root.data->tags.at("int array") == file.data->tags.at("int array")));

  assert(("test_long_array",
      root.data->tags.at("long array") == file.data->tags.at("long array")));


  std::stringstream print_buffer;
  print_buffer << root;

  const std::string printed {print_buffer.str()};

  assert(("printed_byte_array",
      printed.find("byte array: <TagByteArray> {0, 1, 2, 3}") != printed.npos));

  assert(("printed_int_array",
      printed.find("int array: <TagIntArray> {0, 65536, 131072, 196608}") !=
          printed.npos));

  assert(("printed_long_array",
      printed.find("long array: <TagLongArray> {0, 4294967296, 8589934592, "
                   "12884901888}") != printed.npos));
}
