#include <fstream>
#include <sstream>

#undef NDEBUG
#include <cassert>

#include "nbt.hpp"

int main() {

  nbt::TagList test_list {
      nbt::TagList {
          nbt::TagByte {1},
          nbt::TagByte {2},
          nbt::TagByte {3},
      },
      {
          nbt::TagShort {1 << 8},
          nbt::TagShort {2 << 8},
          nbt::TagShort {3 << 8},
      },
      {
          nbt::TagInt {1 << 16},
          nbt::TagInt {2 << 16},
          nbt::TagInt {3 << 16},
      },
      {
          nbt::TagLong {1LL << 32},
          nbt::TagLong {2LL << 32},
          nbt::TagLong {3LL << 32},
      },
      {
          nbt::TagFloat {0.1f},
          nbt::TagFloat {0.2f},
          nbt::TagFloat {0.3f},
      },
      {
          nbt::TagDouble {0.1},
          nbt::TagDouble {0.2},
          nbt::TagDouble {0.3},
      },
      {
          nbt::TagByteArray {1},
          nbt::TagByteArray {2},
          nbt::TagByteArray {3},
      },
      {
          nbt::TagIntArray {1 << 16},
          nbt::TagIntArray {2 << 16},
          nbt::TagIntArray {3 << 16},
      },
      {
          nbt::TagLongArray {1LL << 32},
          nbt::TagLongArray {2LL << 32},
          nbt::TagLongArray {3LL << 32},
      },
      {
          "String #1",
          "String #2",
          "String #3",
      },
      {
          nbt::TagCompound {{"name", "Compound #1"}},
          nbt::TagCompound {{"name", "Compound #2"}},
          nbt::TagCompound {{"name", "Compound #3"}},
      },
  };

  nbt::NBT root {"List Test", {{"list", test_list}}};

  std::stringstream good_buffer;
  good_buffer << std::ifstream {"list.nbt"}.rdbuf();

  std::stringstream test_buffer;
  root.encode(test_buffer);

  assert(("binary_list", good_buffer.str() == test_buffer.str()));


  nbt::NBT file {good_buffer};

  const auto& a {nbt::get_list<nbt::TagList>(root.at<nbt::TagList>("list"))};
  const auto& b {nbt::get_list<nbt::TagList>(file.at<nbt::TagList>("list"))};


#define TEST(name, type, index)                                               \
  assert((                                                                    \
      #name, nbt::get_list<type>(a[index]) == nbt::get_list<type>(b[index])))

  TEST(list_byte, nbt::TagByte, 0);
  TEST(list_short, nbt::TagShort, 1);
  TEST(list_int, nbt::TagInt, 2);
  TEST(list_long, nbt::TagLong, 3);
  TEST(list_float, nbt::TagFloat, 4);
  TEST(list_double, nbt::TagDouble, 5);
  TEST(list_byte_array, nbt::TagByteArray, 6);
  TEST(list_int_array, nbt::TagIntArray, 7);
  TEST(list_long_array, nbt::TagLongArray, 8);


  const auto& dicts_a {nbt::get_list<nbt::TagCompound>(a[10])};
  const auto& dicts_b {nbt::get_list<nbt::TagCompound>(b[10])};

  assert(("compound_list_length", dicts_a.size() == dicts_b.size()));

  for(size_t i {0}; i < dicts_a.size(); i++)
    for(const auto& [key, val] : dicts_a[i].base)
      assert(("compound_compare",
          std::get<nbt::TagString>(val) ==
              std::get<nbt::TagString>(dicts_b[i].at(key))));


  std::stringstream print_buffer;
  print_buffer << root;
  std::stringstream good_print;
  good_print << std::ifstream {"printed_list.txt"}.rdbuf();

  assert(("printed_list", print_buffer.str() == good_print.str()));
}
