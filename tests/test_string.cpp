#include <fstream>
#include <sstream>

#undef NDEBUG
#include <cassert>

#include "nbt.hpp"

int main() {

  nbt::TagString test_string {"This is a string 🙂"};

  nbt::NBT nbt {
      "String Test",
      {
          {"string", test_string},
      },
  };

  std::stringstream good_buffer;
  good_buffer << std::ifstream {"string.nbt", std::ios::binary}.rdbuf();

  std::stringstream test_buffer;
  nbt.encode(test_buffer);

  assert(("binary_string", good_buffer.str() == test_buffer.str()));


  nbt::NBT file {good_buffer};

  assert(("test_string",
      nbt.data->tags.at("string") == file.data->tags.at("string")));

  std::stringstream print_buffer;


  print_buffer << nbt;

  const std::string expected {
      "\"String Test\"\n"
      "<TagCompound> {\n"
      "  string: <TagString> This is a string 🙂\n"
      "}"};

  assert(("printed_string", expected == print_buffer.str()));
}
