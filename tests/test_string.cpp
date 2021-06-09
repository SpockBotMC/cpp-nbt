#include <fstream>
#include <sstream>

#undef NDEBUG
#include <cassert>

#include "nbt.hpp"

int main() {

  nbt::TagString test_string {"This is a string 🙂"};

  nbt::NBT root {"String Test",
      {
          {"string", test_string},
      }};

  std::stringstream good_buffer;
  good_buffer << std::ifstream {"string.nbt", std::ios::binary}.rdbuf();

  std::stringstream test_buffer;
  root.encode(test_buffer);

  assert(("binary_string", !good_buffer.str().compare(test_buffer.str())));


  nbt::NBT file {good_buffer};

  assert(("test_string",
      root.at<nbt::TagString>("string") == file.at<nbt::TagString>("string")));

  std::stringstream print_buffer;
  print_buffer << root;
  const std::string expected {"\"String Test\"\n"
                              "<TagCompound> {\n"
                              "  string: \"This is a string 🙂\"\n"
                              "}"};

  assert(("printed_string", !expected.compare(print_buffer.str())));
}
