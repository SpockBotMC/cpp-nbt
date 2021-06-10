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

  nbt::NBT root {"Numeric Test",
      {
          {"byte", test_byte},
          {"short", test_short},
          {"int", test_int},
          {"long", test_long},
          {"float", test_float},
          {"double", test_double},
      }};


  std::stringstream good_buffer;
  good_buffer << std::ifstream {"numerics.nbt"}.rdbuf();

  std::stringstream test_buffer;
  root.encode(test_buffer);

  assert(("binary_numerics", good_buffer.str() == test_buffer.str()));


  nbt::NBT file {good_buffer};

  assert(("test_byte",
      root.at<nbt::TagByte>("byte") == file.at<nbt::TagByte>("byte")));

  assert(("test_short",
      root.at<nbt::TagShort>("short") == file.at<nbt::TagShort>("short")));

  assert((
      "test_int", root.at<nbt::TagInt>("int") == file.at<nbt::TagInt>("int")));

  assert(("test_long",
      root.at<nbt::TagLong>("long") == file.at<nbt::TagLong>("long")));

  assert(("test_float",
      root.at<nbt::TagFloat>("float") == file.at<nbt::TagFloat>("float")));

  assert(("test_double",
      root.at<nbt::TagDouble>("double") == file.at<nbt::TagDouble>("double")));


  std::stringstream print_buffer;
  print_buffer << root;
  const std::string printed {print_buffer.str()};

  assert(("printed_byte", printed.find("byte: <TagByte> 1") != printed.npos));

  assert((
      "printed_short", printed.find("short: <TagShort> 256") != printed.npos));

  assert(("printed_int", printed.find("int: <TagInt> 65536") != printed.npos));

  assert(("printed_long",
      printed.find("long: <TagLong> 4294967296") != printed.npos));

  assert(("printed_long",
      printed.find("long: <TagLong> 4294967296") != printed.npos));

  assert((
      "printed_float", printed.find("float: <TagFloat> 0.1") != printed.npos));

  assert(("printed_double",
      printed.find("double: <TagDouble> 0.2") != printed.npos));
}
