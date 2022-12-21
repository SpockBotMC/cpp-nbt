#include <fstream>
#include <sstream>

#undef NDEBUG
#include <cassert>

#include "nbt.hpp"

int main() {

  nbt::TagByte test_byte {1};
  nbt::TagShort test_short {1 << 8};
  nbt::TagInt test_int {1 << 16};
  nbt::TagLong test_long {1LL << 32};

  nbt::TagFloat test_float {0.1f};
  nbt::TagDouble test_double {0.2};

  nbt::NBT nbt {
      "Numeric Test",
      {
          {"byte", test_byte},
          {"short", test_short},
          {"int", test_int},
          {"long", test_long},
          {"float", test_float},
          {"double", test_double},
      },
  };


  std::stringstream good_buffer;
  good_buffer << std::ifstream {"numerics.nbt"}.rdbuf();

  std::stringstream test_buffer;
  nbt.encode(test_buffer);

  assert(("binary_numerics", good_buffer.str() == test_buffer.str()));


  nbt::NBT file {good_buffer};

  assert(
      ("test_byte", nbt.data->tags.at("byte") == file.data->tags.at("byte")));

  assert(("test_short",
      nbt.data->tags.at("short") == file.data->tags.at("short")));

  assert(("test_int", nbt.data->tags.at("int") == file.data->tags.at("int")));

  assert(
      ("test_long", nbt.data->tags.at("long") == file.data->tags.at("long")));

  assert(("test_float",
      nbt.data->tags.at("float") == file.data->tags.at("float")));

  assert(("test_double",
      nbt.data->tags.at("double") == file.data->tags.at("double")));


  std::stringstream print_buffer;
  print_buffer << nbt;
  const std::string printed {print_buffer.str()};

  assert(("printed_byte", printed.find("byte: <TagByte> 1") != printed.npos));

  assert(
      ("printed_short", printed.find("short: <TagShort> 256") != printed.npos));

  assert(("printed_int", printed.find("int: <TagInt> 65536") != printed.npos));

  assert(("printed_long",
      printed.find("long: <TagLong> 4294967296") != printed.npos));

  assert(("printed_long",
      printed.find("long: <TagLong> 4294967296") != printed.npos));

  assert(
      ("printed_float", printed.find("float: <TagFloat> 0.1") != printed.npos));

  assert(("printed_double",
      printed.find("double: <TagDouble> 0.2") != printed.npos));
}
