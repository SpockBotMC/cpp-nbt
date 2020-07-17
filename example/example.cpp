#include <iostream>
#include "nbt.hpp"
#include "zstr.hpp"

int main(void) {
  std::ifstream infile("hello_world.nbt");
  auto tc = nbt::read_compound(infile);
  std::cout << tc;
  infile.close();

  zstr::ifstream zin("bigtest.nbt");
  tc.decode_full(zin);
  std::cout << tc;

  std::ofstream outfile("out.nbt");
  tc.encode_full(outfile);
  outfile.close();

  infile.open("out.nbt");
  tc.decode_full(infile);
  std::cout << tc;
  infile.close();
}
