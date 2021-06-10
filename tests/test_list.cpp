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

  assert(!good_buffer.str().compare(test_buffer.str()));
}
