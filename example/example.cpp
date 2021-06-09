#include "nbt.hpp"
#include "zstr.hpp"
#include <iostream>

int main(void) {
  std::ifstream ifs {"hello_world.nbt"};
  nbt::NBT tags {ifs};
  std::cout << tags << std::endl;
  ifs.close();

  zstr::ifstream zfs {"bigtest.nbt"};
  tags.decode(zfs);
  std::cout << tags << std::endl;

  std::ofstream ofs {"out.nbt"};
  tags.name = "Even More Test";
  tags["intArrayTest"] = nbt::TagIntArray {0, 1, 2, 3, 4};
  tags["longArrayTest"] = nbt::TagLongArray {5, 6, 7, 8};
  tags["funBlockGame"] = nbt::TagCompound {
      {"a", nbt::TagDouble {0.1}},
      {"b", nbt::TagFloat {0.2}},
      {"c", nbt::TagString {"Minecraft"}},
  };
  tags.encode(ofs);

  ifs.open("out.nbt");
  tags.decode(ifs);
  std::cout << tags << std::endl;
}
