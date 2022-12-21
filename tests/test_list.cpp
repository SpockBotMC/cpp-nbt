#include <fstream>
#include <sstream>

#undef NDEBUG
#include <cassert>

#include "nbt.hpp"

int main() {

  nbt::TagList test_list {std::vector<nbt::TagList> {
      std::vector<nbt::TagEnd> {{}},
      std::vector<nbt::TagByte> {
          1,
          2,
          3,
      },
      std::vector<nbt::TagShort> {
          1 << 8,
          2 << 8,
          3 << 8,
      },
      std::vector<nbt::TagInt> {
          1 << 16,
          2 << 16,
          3 << 16,
      },
      std::vector<nbt::TagLong> {
          1LL << 32,
          2LL << 32,
          3LL << 32,
      },
      std::vector<nbt::TagFloat> {
          0.1f,
          0.2f,
          0.3f,
      },
      std::vector<nbt::TagDouble> {
          0.1f,
          0.2f,
          0.3f,
      },
      std::vector<nbt::TagByteArray> {
          {1},
          {2},
          {3},
      },
      std::vector<nbt::TagIntArray> {
          {1 << 16},
          {2 << 16},
          {3 << 16},
      },
      std::vector<nbt::TagLongArray> {
          {1LL << 32},
          {2LL << 32},
          {3LL << 32},
      },
      std::vector<nbt::TagString> {
          "String #1",
          "String #2",
          "String #3",
      },
      std::vector<nbt::TagCompound> {
          {{"name", "Compound #1"}},
          {{"name", "Compound #2"}},
          {{"name", "Compound #3"}},
      },
  }};

  nbt::NBT nbt {"List Test", {{"list", test_list}}};

  std::stringstream good_buffer;
  good_buffer << std::ifstream {"list.nbt"}.rdbuf();

  std::stringstream test_buffer;
  nbt.encode(test_buffer);

  assert(("binary_list", good_buffer.str() == test_buffer.str()));

  nbt::NBT file {good_buffer};

  assert(nbt.data->tags.at("list") == file.data->tags.at("list"));


  std::stringstream print_buffer;
  print_buffer << nbt;
  std::stringstream good_print;

  good_print << std::ifstream {"printed_list.txt"}.rdbuf();

  assert(("printed_list", print_buffer.str() == good_print.str()));
}
