/*
Draw a crazy picture,
Write a nutty poem,
Sing a mumble-grumble song,
Whistle through your comb.
Do a loony-goony dance
'Cross the kitchen floor,
Put something silly in the world
That ain't been there before.
    - Shel Silverstein

cpp-nbt
The C++20 NBT serialization header no one asked for
https://github.com/SpockBotMC/cpp-nbt
Licensed under zlib, see repo License file for details
*/

#ifndef NBT_HPP
#define NBT_HPP

#ifndef NBT_MAP_TYPE
#include <boost/container/map.hpp>
#define NBT_MAP_TYPE boost::container::map
#endif

#include <bit>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <variant>
#include <vector>


namespace nbt {

static constexpr std::string_view indent_step {"  "};

enum TagType {
  TAG_END,
  TAG_BYTE,
  TAG_SHORT,
  TAG_INT,
  TAG_LONG,
  TAG_FLOAT,
  TAG_DOUBLE,
  TAG_BYTE_ARRAY,
  TAG_STRING,
  TAG_LIST,
  TAG_COMPOUND,
  TAG_INT_ARRAY,
  TAG_LONG_ARRAY
};


typedef std::nullptr_t TagEnd;


typedef std::int8_t TagByte;
typedef std::int16_t TagShort;
typedef std::int32_t TagInt;
typedef std::int64_t TagLong;


typedef float TagFloat;
typedef double TagDouble;


typedef std::vector<TagByte> TagByteArray;
typedef std::vector<TagInt> TagIntArray;
typedef std::vector<TagLong> TagLongArray;

typedef std::string TagString;

struct TagList;

struct Tag;
typedef NBT_MAP_TYPE<TagString, Tag> TagCompound;

namespace detail {

template <typename T> consteval std::string_view tag_name() {
  if constexpr(std::same_as<T, TagEnd>)
    return "TagEnd";
  else if constexpr(std::same_as<T, TagByte>)
    return "TagByte";
  else if constexpr(std::same_as<T, TagShort>)
    return "TagShort";
  else if constexpr(std::same_as<T, TagInt>)
    return "TagInt";
  else if constexpr(std::same_as<T, TagLong>)
    return "TagLong";
  else if constexpr(std::same_as<T, TagFloat>)
    return "TagFloat";
  else if constexpr(std::same_as<T, TagDouble>)
    return "TagDouble";
  else if constexpr(std::same_as<T, TagByteArray>)
    return "TagByteArray";
  else if constexpr(std::same_as<T, TagString>)
    return "TagString";
  else if constexpr(std::same_as<T, TagList>)
    return "TagList";
  else if constexpr(std::same_as<T, TagCompound>)
    return "TagCompound";
  else if constexpr(std::same_as<T, TagIntArray>)
    return "TagIntArray";
  else if constexpr(std::same_as<T, TagLongArray>)
    return "TagLongArray";
  else
    return "UnknownTag";
}

auto nbeswap(std::integral auto val) noexcept {
  if constexpr(std::endian::native == std::endian::big)
    return val;
  else
    return std::byteswap(val);
}

void encode(std::ostream& os, std::integral auto val) {
  auto out {nbeswap(val)};
  os.write(reinterpret_cast<char*>(&out), sizeof(out));
}

template <std::integral T> T decode(std::istream& is) {
  T val;
  is.read(reinterpret_cast<char*>(&val), sizeof(val));
  return nbeswap(val);
}

void encode(std::ostream& os, std::floating_point auto val) {
  using I = std::conditional_t<sizeof(val) <= sizeof(TagInt), TagInt, TagLong>;
  encode(os, std::bit_cast<I, decltype(val)>(val));
}

template <std::floating_point T> T decode(std::istream& is) {
  using I = std::conditional_t<sizeof(T) <= sizeof(TagInt), TagInt, TagLong>;
  return std::bit_cast<T, I>(decode<I>(is));
}

template <typename T>
concept isTagEnd = std::same_as<T, TagEnd> || std::same_as<T, TagEnd&>;

void encode(std::ostream& os, isTagEnd auto = nullptr) {
  os.put(TAG_END);
} // namespace detail

template <isTagEnd T> T decode(std::istream& is) {
  is.ignore();
  return {};
}

void print(std::ostream& os, isTagEnd auto = nullptr) {
  os << ".";
}

template <typename T>
concept isTagString = std::same_as<T, TagString>;

void encode(std::ostream& os, const isTagString auto& str) {
  encode<TagShort>(os, str.size());
  os.write(str.data(), str.size());
}

template <isTagString T> T decode(std::istream& is) {
  auto len {decode<TagShort>(is)};
  TagString str(len, '\0');
  is.read(str.data(), len);
  return str;
}

template <typename T>
concept isPrint = std::integral<T> || std::floating_point<T> || isTagString<T>;

void print(std::ostream& os, const isPrint auto& val) {
  if constexpr(std::same_as<std::decay_t<decltype(val)>, TagByte>)
    os << static_cast<int>(val);
  else
    os << val;
}

template <typename T>
concept TagArray = std::same_as<T, TagByteArray> ||
    std::same_as<T, TagIntArray> || std::same_as<T, TagLongArray>;

void encode(std::ostream& os, const TagArray auto& vec) {
  encode<TagInt>(os, vec.size());
  for(auto el : vec)
    encode(os, el);
}

template <TagArray T> T decode(std::istream& is) {
  T vec(decode<TagInt>(is));
  for(auto& el : vec)
    el = decode<typename T::value_type>(is);
  return vec;
}

void print_vec(std::ostream& os, const auto& vec) {
  os << "{";
  if(size_t size {vec.size()}; size) {
    os << +vec[0];
    size_t end {size < 7 ? size : 3};
    for(size_t i {1}; i < end; i++)
      os << ", " << +vec[i];
    if(size >= 7)
      os << ", and " << size - 3 << " more";
  }
  os << "}";
}

void print(std::ostream& os, const TagArray auto& vec) {
  print_vec(os, vec);
}

template <typename T>
concept isTagList = std::same_as<T, TagList>;

template <typename T> void encode(std::ostream& os,
    const T& val) requires isTagList<T> || std::same_as<T, Tag> {
  val.encode(os);
}

void print(std::ostream& os, const isTagList auto& val,
    const std::string& indent = "") {
  val.print(os, indent);
}

template <isTagList T> T decode(std::istream& is);

template <typename T>
concept isTagCompound = std::same_as<T, TagCompound>;

void encode(std::ostream& os, const isTagCompound auto& map);

template <isTagCompound T> T decode(std::istream& is);

void print(std::ostream& os, const isTagCompound auto& map,
    const std::string& indent = "");

} // namespace detail

struct TagList
    : public std::variant<std::vector<TagEnd>, std::vector<TagByte>,
          std::vector<TagShort>, std::vector<TagInt>, std::vector<TagLong>,
          std::vector<TagFloat>, std::vector<TagDouble>,
          std::vector<TagByteArray>, std::vector<TagString>,
          std::vector<TagList>, std::vector<TagCompound>,
          std::vector<TagIntArray>, std::vector<TagLongArray>> {
  using variant::variant;

  TagList(std::istream& is) {
    decode(is);
  }

  void encode(std::ostream& os) const {
    detail::encode<TagByte>(os, index());
    auto enc = [&](const auto& vec) {
      detail::encode<TagInt>(os, vec.size());
      for(const auto& val : vec)
        detail::encode(os, val);
    };
    std::visit(enc, *static_cast<const variant*>(this));
  }

  void decode(std::istream& is) {
    switch(detail::decode<TagByte>(is)) { // clang-format off
      case TAG_END:        *this = dec_vec<TagEnd>(is);       break;
      case TAG_BYTE:       *this = dec_vec<TagByte>(is);      break;
      case TAG_SHORT:      *this = dec_vec<TagShort>(is);     break;
      case TAG_INT:        *this = dec_vec<TagInt>(is);       break;
      case TAG_LONG:       *this = dec_vec<TagLong>(is);      break;
      case TAG_FLOAT:      *this = dec_vec<TagFloat>(is);     break;
      case TAG_DOUBLE:     *this = dec_vec<TagDouble>(is);    break;
      case TAG_BYTE_ARRAY: *this = dec_vec<TagByteArray>(is); break;
      case TAG_STRING:     *this = dec_vec<TagString>(is);    break;
      case TAG_LIST:       *this = dec_vec<TagList>(is);      break;
      case TAG_COMPOUND:   *this = dec_vec<TagCompound>(is);  break;
      case TAG_INT_ARRAY:  *this = dec_vec<TagIntArray>(is);  break;
      case TAG_LONG_ARRAY: *this = dec_vec<TagLongArray>(is); break;
      default:             throw std::runtime_error {"invalid tag type"};
    } // clang-format on
  }

  void print(std::ostream& os, const std::string& indent) const {
    os << "<TagList of ";
    std::string next_indent {indent + std::string {indent_step}};

    auto pr = [&]<typename T>(const T& vec) {
      using type = typename T::value_type;
      os << detail::tag_name<type>() << "> {";
      if constexpr(detail::isTagList<type> || detail::isTagCompound<type>) {
        for(const auto& tag : vec) {
          os << "\n" << next_indent;
          detail::print(os, tag, next_indent);
        }
      } else if constexpr(detail::TagArray<type>) {
        for(const auto& tag : vec) {
          os << "\n" << next_indent;
          detail::print_vec(os, tag);
        }
      } else if constexpr(detail::isTagEnd<type>) {
        os << "}";
      } else if constexpr(detail::isTagString<type>) {
        for(const auto& tag : vec) {
          os << "\n" << next_indent;
          detail::print(os, tag);
        }
      } else {
        os << "\n" << next_indent;
        detail::print_vec(os, vec);
      }
      os << "\n" << indent << "}";
    };
    std::visit(pr, *static_cast<const variant*>(this));
  }

private:
  template <typename T> std::vector<T> dec_vec(std::istream& is) {
    std::vector<T> vec(detail::decode<TagInt>(is));
    for(auto& el : vec)
      el = detail::decode<T>(is);
    return vec;
  }
};

struct Tag : public std::variant<TagEnd, TagByte, TagShort, TagInt, TagLong,
                 TagFloat, TagDouble, TagByteArray, TagString, TagList,
                 TagCompound, TagIntArray, TagLongArray> {
  using variant::variant;

  Tag(std::istream& is, size_t type) {
    decode(is, type);
  }

  void encode(std::ostream& os) const {
    auto enc = [&](const auto& val) { detail::encode(os, val); };
    std::visit(enc, *static_cast<const variant*>(this));
  }

  void decode(std::istream& is, size_t type) {
    switch(type) { // clang-format off
      case TAG_END:        *this = detail::decode<TagEnd>(is);       break;
      case TAG_BYTE:       *this = detail::decode<TagByte>(is);      break;
      case TAG_SHORT:      *this = detail::decode<TagShort>(is);     break;
      case TAG_INT:        *this = detail::decode<TagInt>(is);       break;
      case TAG_LONG:       *this = detail::decode<TagLong>(is);      break;
      case TAG_FLOAT:      *this = detail::decode<TagFloat>(is);     break;
      case TAG_DOUBLE:     *this = detail::decode<TagDouble>(is);    break;
      case TAG_BYTE_ARRAY: *this = detail::decode<TagByteArray>(is); break;
      case TAG_STRING:     *this = detail::decode<TagString>(is);    break;
      case TAG_LIST:       *this = detail::decode<TagList>(is);      break;
      case TAG_COMPOUND:   *this = detail::decode<TagCompound>(is);  break;
      case TAG_INT_ARRAY:  *this = detail::decode<TagIntArray>(is);  break;
      case TAG_LONG_ARRAY: *this = detail::decode<TagLongArray>(is); break;
      default:             throw std::runtime_error {"invalid tag type"};
    } //clang-format on
  }

  void print(std::ostream& os) {
      auto pr = [&](const auto& val){detail::print(os, val);};
      std::visit(pr, *static_cast<const variant*>(this));
  }

private:
  friend std::ostream& operator << (std::ostream& os, Tag val) {
    val.print(os);
    return os;
  }
};

namespace detail {

template <isTagList T> T decode(std::istream& is) {
  return TagList {is};
}

void encode(std::ostream& os, const isTagCompound auto& map) {
  for(const auto& [key, tag] : map) {
    detail::encode<TagByte>(os, tag.index());
    detail::encode(os, key);
    detail::encode(os, tag);
  }
  detail::encode<TagByte>(os, TAG_END);
}

template <isTagCompound T> T decode(std::istream& is) {
  TagCompound map;
  auto type {decode<TagByte>(is)};
  for(; type != TAG_END; type = decode<TagByte>(is)) {
    auto key {decode<TagString>(is)};
    map[key] = Tag {is, static_cast<size_t>(type)};
  }
  return map;
}

void print(std::ostream& os, const isTagCompound auto& map,
    const std::string& indent) {
  std::string next_indent {indent + std::string{indent_step}};
  os << "<TagCompound> {";
  auto pr = [&]<typename T>(const T& tag) {
    if constexpr(detail::isTagList<T> || detail::isTagCompound<T>) {
      print(os, tag, next_indent);
    } else {
      os << "<" << detail::tag_name<T>() << "> ";
      print(os, tag);
    }
  };
  for(const auto& [name, tag] : map) {
    os << "\n" << next_indent << name << ": ";
    std::visit(pr, tag);
  }
  if(map.empty())
    os << "}";
  else
    os << "\n" << indent << "}";
}

} // namespace detail

struct NBTData {
  TagString name;
  TagCompound tags;
};

struct NBT {
  std::optional<NBTData> data;

  NBT() = default;

  NBT(const TagString& name, const TagCompound& tags = {})
      : data {{name, tags}} {}
  NBT(const TagCompound& tags, const TagString& name = {})
      : data {{name, tags}} {}

  NBT(std::istream &is) {
    decode(is);
  }
  NBT(std::istream &&is) {
    decode(is);
  }

  void decode(std::istream& is) {
    auto type {detail::decode<TagByte>(is)};
    if(type == TAG_COMPOUND) {
      auto name {detail::decode<TagString>(is)};
      data = NBTData {std::move(name), detail::decode<TagCompound>(is)};
    } else if(type != TAG_END) {
      throw std::runtime_error {"invalid tag type"};
    }
  }

  void encode(std::ostream& os) const {
    if(data) {
      detail::encode<TagByte>(os, TAG_COMPOUND);
      detail::encode(os, data->name);
      detail::encode(os, data->tags);
    } else {
      detail::encode<TagEnd>(os);
    }
  }

  void print(std::ostream& os) const {
    if(data) {
      os << "\"" << data->name << "\"\n";
      detail::print(os, data->tags);
    } else {
      os << "Empty NBT";
    }
  }

private:
  friend std::ostream& operator << (std::ostream& os, NBT val) {
    val.print(os);
    return os;
  }
};

} // namespace nbt

#endif // NBT_HPP
