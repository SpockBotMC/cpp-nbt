#include <cstring>
#include <fstream>
#include <sstream>

#undef NDEBUG
#include <cassert>

#include "nbt.hpp"

int main() {

  // TagCompound store/get
  nbt::TagCompound tag_compound {
      {"Hello", "World"},
  };

  std::ostringstream {} << tag_compound;

  // NBT default constructor
  std::ostringstream os {};
  nbt::NBT {}.encode(os);

  // NBT name constructor and named empty encode
  nbt::NBT {"NBT"}.encode(os);

  // NBT r-value reference constructor and invalid tag exception
  try {
    nbt::NBT {std::istringstream {"not a tag"}};
  } catch(std::exception&) {
  }

  // NBT TagCompound constructor
  nbt::NBT {tag_compound};

  nbt::TagCompound invalid;
  struct make_invalid_int {
    nullptr_t val;
    operator int() {
      throw std::runtime_error {"now you're invalid"};
    }
  };

  try {
    invalid["invalid_field"] = make_invalid_int {};
  } catch(std::exception&) {
  }

  try {
    std::stringstream os {};
    nbt::detail::encode(os, invalid);
  } catch(std::exception&) {
  }

  try {
    std::ostringstream {} << invalid;
  } catch(std::exception&) {
  }

  std::stringstream invalid_buf {};

  nbt::detail::encode<nbt::TagByte>(invalid_buf, nbt::TAG_COMPOUND);
  nbt::detail::encode<nbt::TagString>(invalid_buf, "invalid_nbt");
  nbt::detail::encode<nbt::TagByte>(invalid_buf, -1);
  nbt::detail::encode<nbt::TagByte>(invalid_buf, nbt::TAG_END);

  try {
    nbt::NBT {invalid_buf};
  } catch(std::exception&) {
  }

  std::stringstream multi_TagEnd_list {};
  nbt::detail::encode<nbt::TagByte>(multi_TagEnd_list, nbt::TAG_END);
  nbt::detail::encode<nbt::TagInt>(multi_TagEnd_list, 10);
  nbt::TagList end_list {nbt::detail::decode<nbt::TagList>(multi_TagEnd_list)};

  std::stringstream invalid_list_buf {};

  nbt::detail::encode<nbt::TagByte>(invalid_list_buf, -1);
  nbt::detail::encode<nbt::TagInt>(invalid_list_buf, 10);

  try {
    nbt::detail::decode<nbt::TagList>(invalid_list_buf);
  } catch(std::exception&) {
  }
};
