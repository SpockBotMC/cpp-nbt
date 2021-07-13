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

#include <bit>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>


namespace nbt {

inline static std::string indent_step {"  "};


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
struct TagCompound;


typedef std::variant<TagEnd, TagByte, TagShort, TagInt, TagLong, TagFloat,
    TagDouble, TagByteArray, TagString, TagList, TagCompound, TagIntArray,
    TagLongArray>
    Tag;


namespace detail {
void print_list(std::ostream& os, const std::string& indent,
    const TagList& list);

void print_compound(std::ostream& os, const std::string& indent,
    const TagCompound& map);
} // namespace detail


struct TagList {
  TagList() = default;
  template <typename T> TagList(const std::vector<T>& base) : base {base} {}
  template <typename T> TagList(std::initializer_list<T> lst)
      : base {std::in_place_type<std::vector<T>>, lst} {}
  TagList(std::initializer_list<const char*> lst)
      : base {std::in_place_type<std::vector<TagString>>, lst.begin(),
            lst.end()} {}

  std::variant<std::vector<TagEnd>, std::vector<TagByte>,
      std::vector<TagShort>, std::vector<TagInt>, std::vector<TagLong>,
      std::vector<TagFloat>, std::vector<TagDouble>, std::vector<TagByteArray>,
      std::vector<TagString>, std::vector<TagList>, std::vector<TagCompound>,
      std::vector<TagIntArray>, std::vector<TagLongArray>>
      base;

  size_t index() const {
    return base.index();
  }

  template <typename T> TagList& operator=(const std::vector<T>& other) {
    base = other;
    return *this;
  }

private:
  friend std::ostream& operator<<(std::ostream& os, const TagList& tag) {
    detail::print_list(os, "", tag);
    return os;
  }
};

template <typename T> const std::vector<T>& get_list(const TagList& list) {
  return std::get<std::vector<T>>(list.base);
}
template <typename T> std::vector<T>& get_list(TagList& list) {
  return std::get<std::vector<T>>(list.base);
}


struct TagCompound {
  TagCompound() = default;
  TagCompound(
      std::initializer_list<std::pair<const std::string, nbt::Tag>> lst)
      : base {lst} {}
  // std::map of an incomplete type is technically UB, but as a practical
  // matter works everywhere. The same cannot be said of std::unordered_map
  std::map<std::string, Tag> base;

  Tag& operator[](const std::string& key) {
    return base[key];
  }
  Tag& operator[](const char* key) {
    return base[key];
  }

  Tag& at(const std::string& key) {
    return base.at(key);
  }
  const Tag& at(const std::string& key) const {
    return base.at(key);
  }

  template <typename T> T& at(const std::string& key) {
    return std::get<T>(base.at(key));
  }

private:
  friend std::ostream& operator<<(std::ostream& os, const TagCompound& tag) {
    detail::print_compound(os, "", tag);
    return os;
  }
};

struct NBT : public TagCompound {
  NBT() = default;
  NBT(std::istream& buf) {
    decode(buf);
  };
  NBT(std::istream&& buf) {
    decode(buf);
  }
  NBT(const TagCompound& tag) : TagCompound {tag} {}
  NBT(const std::string& name) : name {name} {}
  NBT(const std::string& name, const TagCompound& tag)
      : TagCompound {tag}, name {name} {}

  std::optional<std::string> name;

  void decode(std::istream& buf);
  void decode(std::istream&& buf) {
    decode(buf);
  }

  void encode(std::ostream& buf) const;
  void encode(std::ostream&& buf) const {
    encode(buf);
  }

  operator bool() const {
    return !name && base.empty();
  }

private:
  friend std::ostream& operator<<(std::ostream& os, const NBT& val) {
    os << "\"" << (val.name ? *val.name : "") << "\"\n";
    detail::print_compound(os, "", val);
    return os;
  }
};


namespace detail {


#if defined(_MSC_VER)

inline auto bswap(std::uint64_t v) noexcept {
  return _byteswap_uint64(v);
}
inline auto bswap(std::uint32_t v) noexcept {
  return _byteswap_ulong(v);
}
inline auto bswap(std::uint16_t v) noexcept {
  return _byteswap_ushort(v);
}

#else

inline auto bswap(std::uint64_t v) noexcept {
  return __builtin_bswap64(v);
}
inline auto bswap(std::uint32_t v) noexcept {
  return __builtin_bswap32(v);
}
inline auto bswap(std::uint16_t v) noexcept {
  return __builtin_bswap16(v);
}

#endif // _MSC_VER

inline auto byteswap(std::integral auto val) noexcept {
  if constexpr(sizeof(val) == 1)
    return static_cast<std::make_unsigned_t<decltype(val)>>(val);
  else
    return bswap(static_cast<std::make_unsigned_t<decltype(val)>>(val));
}

inline auto nbeswap(std::integral auto val) noexcept {
  if constexpr(std::endian::native == std::endian::big)
    return val;
  return byteswap(val);
}


template <std::integral T> T decode(std::istream& buf) {
  T val;
  buf.read(reinterpret_cast<char*>(&val), sizeof(val));
  return nbeswap(val);
}

template <std::integral T> void encode(std::ostream& buf, const T val) {
  std::make_unsigned_t<T> out {nbeswap(val)};
  buf.write(reinterpret_cast<char*>(&out), sizeof(out));
}

template <std::floating_point T> T decode(std::istream& buf) {
  std::conditional_t<sizeof(T) <= sizeof(TagInt), TagInt, TagLong> in;
  buf.read(reinterpret_cast<char*>(&in), sizeof(in));
  in = nbeswap(in);
  return std::bit_cast<T, decltype(in)>(in);
}

template <std::floating_point T> void encode(std::ostream& buf, const T val) {
  std::conditional_t<sizeof(T) <= sizeof(TagInt), TagInt, TagLong> out {
      std::bit_cast<decltype(out), T>(val)};
  out = nbeswap(out);
  buf.write(reinterpret_cast<char*>(&out), sizeof(out));
}

template <std::integral T> std::vector<T> decode_array(std::istream& buf) {
  std::int32_t len;
  buf.read(reinterpret_cast<char*>(&len), sizeof(len));
  std::vector<T> vec(nbeswap(len));
  for(auto& el : vec) {
    buf.read(reinterpret_cast<char*>(&el), sizeof(el));
    el = nbeswap(el);
  }
  return vec;
}

template <std::integral T>
void encode_array(std::ostream& buf, const std::vector<T>& vec) {
  std::uint32_t len {nbeswap(static_cast<std::int32_t>(vec.size()))};
  buf.write(reinterpret_cast<char*>(&len), sizeof(len));
  for(auto el : vec) {
    el = nbeswap(el);
    buf.write(reinterpret_cast<char*>(&el), sizeof(el));
  }
}

void print_array(std::ostream& os, const auto& vec) {
  os << "{";
  if(const int size {static_cast<int>(vec.size())}; size) {
    os << +vec[0];
    for(int i {1}; i < (size < 7 ? size : 3); i++)
      os << ", " << +vec[i];
    if(size > 7)
      os << ", and " << size - 3 << " more";
  }
  os << "}";
}

inline TagString decode_string(std::istream& buf) {
  std::int16_t len {decode<TagShort>(buf)};
  std::string str(len, '\0');
  buf.read(str.data(), len);
  return str;
}

inline void encode_string(std::ostream& buf, const TagString& str) {
  encode<TagShort>(buf, static_cast<TagShort>(str.size()));
  buf.write(str.data(), str.size());
}

// clang-format off
#define ALL_NUMERIC(macro)                                                    \
  macro(TAG_BYTE, TagByte)                                                    \
  macro(TAG_SHORT, TagShort)                                                  \
  macro(TAG_INT, TagInt)                                                      \
  macro(TAG_LONG, TagLong)                                                    \
  macro(TAG_FLOAT, TagFloat)                                                  \
  macro(TAG_DOUBLE, TagDouble)

#define ALL_ARRAYS(macro)                                                     \
  macro(TAG_BYTE_ARRAY, TagByteArray, TagByte)                                \
  macro(TAG_INT_ARRAY, TagIntArray, TagInt)                                   \
  macro(TAG_LONG_ARRAY, TagLongArray, TagLong)


#define ALL_OTHERS(macro)                                                     \
  macro(TAG_STRING, TagString, _string)                                       \
  macro(TAG_LIST, TagList, _list)                                             \
  macro(TAG_COMPOUND, TagCompound, _compound)
// clang-format on


TagCompound decode_compound(std::istream& buf);
void encode_compound(std::ostream& buf, const TagCompound& map);
void print_compound(std::ostream& os, const std::string& indent,
    const TagCompound& map);

inline TagList decode_list(std::istream& buf) {
  std::int8_t type {decode<TagByte>(buf)};
  std::int32_t len {decode<TagInt>(buf)};
  if(len <= 0)
    return {};

  switch(type) {
    case TAG_END:
      return {};

#define X(enum, type)                                                         \
  case(enum): {                                                               \
    std::vector<type> vec(len);                                               \
    for(auto& val : vec)                                                      \
      val = decode<type>(buf);                                                \
    return vec;                                                               \
  };
      ALL_NUMERIC(X)
#undef X

#define X(enum, type, base_type)                                              \
  case(enum): {                                                               \
    std::vector<type> vec(len);                                               \
    for(auto& val : vec)                                                      \
      val = decode_array<base_type>(buf);                                     \
    return vec;                                                               \
  };
      ALL_ARRAYS(X)
#undef X

#define X(enum, type, ext)                                                    \
  case enum: {                                                                \
    std::vector<type> vec(len);                                               \
    for(auto& val : vec)                                                      \
      val = decode##ext(buf);                                                 \
    return vec;                                                               \
  };
      ALL_OTHERS(X)
#undef X

    default:
      throw std::runtime_error {"invalid tag type"};
  }
}

inline void encode_list(std::ostream& buf, const TagList& list) {
  /*
  if(list.base.valueless_by_exception())
    throw std::runtime_error {"invalid TagList"};
  */
  encode<TagByte>(buf, static_cast<TagByte>(list.index()));
  switch(list.index()) {
    case TAG_END:
      encode<TagInt>(buf, 0);
      break;

#define X(enum, type)                                                         \
  case enum: {                                                                \
    auto& vec {get_list<type>(list)};                                         \
    encode<TagInt>(buf, static_cast<TagInt>(vec.size()));                     \
    for(const auto val : vec)                                                 \
      encode<type>(buf, val);                                                 \
  } break;
      ALL_NUMERIC(X)
#undef X

#define X(enum, type, base_type)                                              \
  case enum: {                                                                \
    auto& vec {get_list<type>(list)};                                         \
    encode<TagInt>(buf, static_cast<TagInt>(vec.size()));                     \
    for(const auto& val : vec)                                                \
      encode_array<base_type>(buf, val);                                      \
  } break;
      ALL_ARRAYS(X)
#undef X

#define X(enum, type, ext)                                                    \
  case enum: {                                                                \
    auto& vec {get_list<type>(list)};                                         \
    encode<TagInt>(buf, static_cast<TagInt>(vec.size()));                     \
    for(const auto& val : vec)                                                \
      encode##ext(buf, val);                                                  \
  } break;
      ALL_OTHERS(X)
#undef X
  }
}

inline void print_list(std::ostream& os, const std::string& indent,
    const TagList& list) {

  os << "<TagList of ";
  std::string next_indent {indent + indent_step};

  switch(list.index()) {
    case TAG_END:
      os << "TagEnd> {}";
      break;

#define X(enum, type)                                                         \
  case enum: {                                                                \
    os << #type "> ";                                                         \
    auto& vec {get_list<type>(list)};                                         \
    print_array(os, vec);                                                     \
  } break;
      ALL_NUMERIC(X)
#undef X

#define X(enum, type, base_type)                                              \
  case enum: {                                                                \
    os << #type "> {";                                                        \
    auto& vec {get_list<type>(list)};                                         \
    if(size_t size {vec.size()}; size) {                                      \
      os << "\n" << next_indent;                                              \
      print_array(os, vec[0]);                                                \
      for(size_t i {1}; i < size; i++) {                                      \
        os << ",\n" << next_indent;                                           \
        print_array(os, vec[i]);                                              \
      }                                                                       \
      os << "\n" << indent << "}";                                            \
    }                                                                         \
  } break;
      ALL_ARRAYS(X)
#undef X

    case TAG_STRING: {
      os << "TagString> {";
      auto& vec {get_list<TagString>(list)};
      if(size_t size {vec.size()}; size) {
        os << "\n" << next_indent << "\"" << vec[0] << "\"";
        for(size_t i {1}; i < size; i++)
          os << ",\n" << next_indent << "\"" << vec[i] << "\"";
        os << "\n" << indent << "}";
      }
    } break;

    case TAG_LIST: {
      os << "TagList> {";
      auto& vec {get_list<TagList>(list)};
      if(size_t size {vec.size()}; size) {
        os << "\n" << next_indent;
        print_list(os, next_indent, vec[0]);
        for(size_t i {1}; i < size; i++) {
          os << ",\n" << next_indent;
          print_list(os, next_indent, vec[i]);
        }
        os << "\n" << indent << "}";
      }
    } break;

    case TAG_COMPOUND: {
      os << "TagCompound> {";
      auto& vec {get_list<TagCompound>(list)};
      if(size_t size {vec.size()}; size) {
        os << "\n" << next_indent;
        print_compound(os, next_indent, vec[0]);
        for(size_t i {1}; i < size; i++) {
          os << ",\n" << next_indent;
          print_compound(os, next_indent, vec[i]);
        }
        os << "\n" << indent << "}";
      }
    } break;
  }
}

inline TagCompound decode_compound(std::istream& buf) {
  TagCompound tag;
  TagByte type {decode<TagByte>(buf)};
  for(; type != TAG_END; type = decode<TagByte>(buf)) {
    std::string key {decode_string(buf)};
    switch(type) {
#define X(enum, type)                                                         \
  case enum:                                                                  \
    tag.base[key] = decode<type>(buf);                                        \
    break;
      ALL_NUMERIC(X)
#undef X

#define X(enum, type, base_type)                                              \
  case enum:                                                                  \
    tag.base[key] = decode_array<base_type>(buf);                             \
    break;
      ALL_ARRAYS(X)
#undef X

#define X(enum, type, ext)                                                    \
  case enum:                                                                  \
    tag.base[key] = decode##ext(buf);                                         \
    break;
      ALL_OTHERS(X)
#undef X
      default:
        throw std::runtime_error {"invalid tag type"};
    }
  }
  return tag;
}

inline void encode_compound(std::ostream& buf, const TagCompound& map) {
  for(const auto& [key, tag] : map.base) {
    encode<TagByte>(buf, static_cast<TagByte>(tag.index()));
    encode_string(buf, key);

    switch(tag.index()) {
#define X(enum, type)                                                         \
  case enum:                                                                  \
    encode<type>(buf, std::get<type>(tag));                                   \
    break;
      ALL_NUMERIC(X)
#undef X

#define X(enum, type, base_type)                                              \
  case enum:                                                                  \
    encode_array<base_type>(buf, std::get<type>(tag));                        \
    break;
      ALL_ARRAYS(X)
#undef X

#define X(enum, type, ext)                                                    \
  case enum:                                                                  \
    encode##ext(buf, std::get<type>(tag));                                    \
    break;
      ALL_OTHERS(X)
#undef X

      default:
        throw std::runtime_error {"invalid tag type"};
    }
  }
  encode<TagByte>(buf, 0);
}

inline void print_compound(std::ostream& os, const std::string& indent,
    const TagCompound& map) {
  os << "<TagCompound> {";
  std::string next_indent {indent + indent_step};
  bool first {true};

  for(const auto& [key, tag] : map.base) {
    if(first)
      first = false;
    else
      os << ",";
    os << "\n" << next_indent << key << ": ";

    switch(tag.index()) {
#define X(enum, type)                                                         \
  case enum:                                                                  \
    os << "<" #type "> " << +std::get<type>(tag);                             \
    break;
      ALL_NUMERIC(X)
#undef X

#define X(enum, type, base_type)                                              \
  case enum:                                                                  \
    print_array(os, std::get<type>(tag));                                     \
    break;
      ALL_ARRAYS(X)
#undef X

      case TAG_STRING:
        os << "\"" << std::get<TagString>(tag) << "\"";
        break;

      case TAG_LIST:
        print_list(os, next_indent, std::get<TagList>(tag));
        break;

      case TAG_COMPOUND:
        print_compound(os, next_indent, std::get<TagCompound>(tag));
        break;

      default:
        throw std::runtime_error {"invalid tag"};
    }
  }
  if(!map.base.empty())
    os << "\n" << indent;
  os << "}";
}

} // namespace detail

inline void NBT::decode(std::istream& buf) {
  TagByte type {nbt::detail::decode<TagByte>(buf)};
  if(type == TAG_COMPOUND) {
    name = detail::decode_string(buf);
    base = detail::decode_compound(buf).base;
  } else if(type != TAG_END)
    throw std::runtime_error {"invalid tag type"};
}

inline void NBT::encode(std::ostream& buf) const {
  if(!name && base.empty())
    detail::encode<TagByte>(buf, TAG_END);
  else {
    detail::encode<TagByte>(buf, TAG_COMPOUND);
    detail::encode_string(buf, name ? *name : "");
    detail::encode_compound(buf, *this);
  }
}

} // namespace nbt

#endif // NBT_HPP
