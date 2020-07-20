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

#include <iostream>
#include <string>
#include <cstdint>
#include <concepts>
#include <vector>
#include <optional>
#include <string.h>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <bit>
#include <type_traits>
#include <cassert>

namespace nbt {

#if defined(_MSC_VER)

inline auto bswap(std::uint64_t v) noexcept { return _byteswap_uint64(v); }
inline auto bswap(std::uint32_t v) noexcept { return _byteswap_ulong(v); }
inline auto bswap(std::uint16_t v) noexcept { return _byteswap_ushort(v); }

#else

inline auto bswap(std::uint64_t v) noexcept { return __builtin_bswap64(v); }
inline auto bswap(std::uint32_t v) noexcept { return __builtin_bswap32(v); }
inline auto bswap(std::uint16_t v) noexcept { return __builtin_bswap16(v); }

#endif // _MSC_VER

//I don't know why this is required but compiles sometimes fail without it
inline auto bswap(std::uint8_t v) noexcept { return v; }

inline auto byteswap(std::integral auto val) noexcept {
  if constexpr(sizeof(val) == 1)
    return static_cast<std::make_unsigned_t<decltype(val)>>(val);
  return bswap(static_cast<std::make_unsigned_t<decltype(val)>>(val));
}

inline auto nbeswap(std::integral auto val) noexcept {
  if constexpr(std::endian::native == std::endian::big)
    return val;
  return byteswap(val);
}

inline auto nleswap(std::integral auto val) noexcept {
  if constexpr(std::endian::native == std::endian::little)
    return val;
  return byteswap(val);
}

// Stupid hack because C++20 still doesn't have string literal template params
template<int N> struct _FixedString {
  constexpr _FixedString(char const (&s)[N]) {
    std::copy_n(s, N, this->el);
  }
  constexpr bool operator==(_FixedString const&) const = default;
  constexpr auto operator<=>(_FixedString const&) const = default;
  char el[N];
};
template<int N> _FixedString(char const(&)[N])->_FixedString<N>;


// My tiny, crap version of pprint
// But we barely need pprint at all, so we use this instead
class NbtPrinter {
  std::ostream &stream_;
  size_t indent_;
  std::string line_terminator_;

public:
  NbtPrinter(std::ostream &stream = std::cout) : stream_(stream),
  indent_(2), line_terminator_("\n") {}

  NbtPrinter& line_terminator(const std::string& value) {
    line_terminator_ = value;
    return *this;
  }
  NbtPrinter& indent(size_t indent) {
    indent_ = indent;
    return *this;
  }
  NbtPrinter& inc_indent(size_t inc) {
    indent_ += inc;
    return *this;
  }
  NbtPrinter& dec_indent(size_t dec) {
    indent_ -= dec;
    return *this;
  }

  void print(auto value) {
    print_internal(value, 0, line_terminator_);
  }
  void print_inline(auto value) {
    print_internal(value, indent_, "");
  }

private:
  void print_internal(auto value, size_t indent = 0,
  const std::string &line_terminator = "\n") {
    stream_ << std::string(indent, ' ') << value << line_terminator;
  }

  void print_internal(float value, size_t indent = 0,
  const std::string &line_terminator = "\n") {
    stream_ << std::string(indent, ' ') << value << 'f' << line_terminator;
  }
};


enum {
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


class BaseTag {
public:
  const std::uint8_t tag_id;
  const std::string type_name;
  std::optional<std::string> name;
  BaseTag(const std::uint8_t id, const std::string type_name) :
  tag_id(id), type_name(type_name) {}
  BaseTag(const std::string name, const std::uint8_t id,
  const std::string type_name) :
  tag_id(id), type_name(type_name), name(name)  {}
  virtual ~BaseTag() = default;

  virtual void encode(std::ostream &buf) const = 0;
  virtual void decode(std::istream &buf) = 0;
  virtual void print(std::ostream &os) const {
    NbtPrinter pr(os);
    print(pr);
  };
  virtual void print(NbtPrinter &pr, int pr_inline = 0) const = 0;
};

static std::ostream& operator<<(std::ostream &os, const BaseTag &b) {
  b.print(os);
  return os;
}


template <typename T, int id>
class Tag : public BaseTag {
public:
  T val;
  // Pass type_name string as param because trying to pass _FixedString between
  // templates is a no go. See: https://stackoverflow.com/q/62874007/1201456
  Tag(const std::string type_name) : BaseTag(id, type_name) {}
  Tag(const std::string name, const::std::string type_name) :
  BaseTag(name, id, type_name) {}
  Tag(const T val, const std::string name, const::std::string type_name) :
  BaseTag(name, id, type_name), val(val) {}
};


template <std::integral Integral, std::uint8_t id, _FixedString type_name>
class TagIntegral : public Tag<Integral, id> {
public:
  TagIntegral() : TagIntegral::Tag(type_name.el) {}
  TagIntegral(std::string name) : TagIntegral::Tag(name, type_name.el) {}
  TagIntegral(Integral val, std::string name) :
  TagIntegral::Tag(val, name, type_name.el) {}
  TagIntegral(Integral val) : TagIntegral::Tag(type_name.el) {
    this->val = val;
  }
  TagIntegral(std::istream &buf) : TagIntegral::Tag(type_name.el) {
    this->decode(buf);
  }
  TagIntegral(std::istream &buf, std::string name) :
  TagIntegral::Tag(name, type_name.el) {
    this->decode(buf);
  }

  void decode(std::istream &buf) {
    buf.read(reinterpret_cast<char *>(&this->val), sizeof(this->val));
    this->val = nbeswap(this->val);
  }

  void encode(std::ostream &buf) const {
    Integral tmp = nbeswap(this->val);
    buf.write(reinterpret_cast<char *>(&tmp), sizeof(tmp));
  }

  void print(NbtPrinter &pr, int pr_inline = 0) const {
    std::string str;
    if(this->name.has_value())
      str = this->type_name + "('" + this->name.value() + "'): " +
      std::to_string(this->val);
    else
      str = this->type_name + "(None): " + std::to_string(this->val);
    if(pr_inline)
      pr.print_inline(str);
    else
      pr.print(str);
  }
};

typedef TagIntegral<std::int8_t, TAG_BYTE, "TagByte"> TagByte;
typedef TagIntegral<std::int16_t, TAG_SHORT, "TagShort"> TagShort;
typedef TagIntegral<std::int32_t, TAG_INT, "TagInt"> TagInt;
typedef TagIntegral<std::int64_t, TAG_LONG, "TagLong"> TagLong;


template <std::floating_point Decimal, std::uint8_t id, _FixedString type_name>
class TagDecimal : public Tag<Decimal, id> {
public:

  TagDecimal() : TagDecimal::Tag(type_name.el) {}
  TagDecimal(std::string name) : TagDecimal::Tag(name, type_name.el) {}
  TagDecimal(Decimal val, std::string name) :
  TagDecimal::Tag(val, name, type_name.el) {}
  TagDecimal(Decimal val) : TagDecimal::Tag(type_name.el) {
    this->val = val;
  }
  TagDecimal(std::istream &buf) : TagDecimal::Tag(type_name.el) {
    this->decode(buf);
  }
  TagDecimal(std::istream &buf, std::string name) :
  TagDecimal::Tag(name, type_name.el) {
    this->decode(buf);
  }

  void decode(std::istream &buf) {
    std::conditional_t<
      std::is_same_v<Decimal, float>, std::uint32_t , std::uint64_t
    > tmp;
    buf.read(reinterpret_cast<char *>(&tmp), sizeof(tmp));
    tmp = nbeswap(tmp);
    memcpy(&this->val, &tmp, sizeof(tmp));
  }

  void encode(std::ostream &buf) const {
    std::conditional_t<
      std::is_same_v<Decimal, float>, std::uint32_t , std::uint64_t
    > tmp;
    memcpy(&tmp, &this->val, sizeof(tmp));
    tmp = nbeswap(tmp);
    buf.write(reinterpret_cast<char *>(&tmp), sizeof(tmp));
  }

  void print(NbtPrinter &pr, int pr_inline = 0) const {
    std::string str;
    if(this->name.has_value())
      str = this->type_name + "('" + this->name.value() + "'): " +
      std::to_string(this->val);
    else
      str = this->type_name + "(None): " + std::to_string(this->val);
    if(pr_inline)
      pr.print_inline(str);
    else
      pr.print(str);
  }
};

typedef TagDecimal<float, TAG_FLOAT, "TagFloat"> TagFloat;
typedef TagDecimal<double, TAG_DOUBLE, "TagDouble"> TagDouble;


template <std::integral T, std::uint8_t id, _FixedString type_name>
class TagArray : public Tag<std::vector<T>, id> {
public:

  TagArray() : TagArray::Tag(type_name.el) {}
  TagArray(std::string name) : TagArray::Tag(name, type_name.el) {}
  TagArray(std::vector<T> val, std::string name) :
  TagArray::Tag(val, name, type_name.el) {}
  TagArray(std::initializer_list<T> l) : TagArray::Tag(type_name.el) {
    this->val = l;
  }
  TagArray(std::vector<T> val) : TagArray::Tag(type_name.el) {
    this->val = val;
  }
  TagArray(std::istream &buf) : TagArray::Tag(type_name.el) {
    this->decode(buf);
  }
  TagArray(std::istream &buf, std::string name) :
  TagArray::Tag(name, type_name.el) {
    this->decode(buf);
  }

  void decode(std::istream &buf) {
    std::int32_t len;
    buf.read(reinterpret_cast<char *>(&len), sizeof(len));
    len = nbeswap(len);
    if(len <= 0) return;
    this->val.resize(len);
    for(auto &&el : this->val) {
      buf.read(reinterpret_cast<char *>(&el), sizeof(el));
      el = nbeswap(el);
    }
  }

  void encode(std::ostream &buf) const {
    std::int32_t len = nbeswap(static_cast<std::int32_t>(this->val.size()));
    buf.write(reinterpret_cast<char *>(&len), sizeof(len));
    for(auto &&el : this->val) {
      auto tmp = nbeswap(el);
      buf.write(reinterpret_cast<char *>(&tmp), sizeof(tmp));
    }
  }

  void print(NbtPrinter &pr, int pr_inline = 0) const {
    std::string str;
    if(this->name.has_value())
      str = this->type_name + "('" + this->name.value() + "'): [";
    else
      str = this->type_name + "(None) : [";
    for(auto &el : this->val)
      str += std::to_string(el) + ", ";
    if(this->val.size())
      str.resize(str.size() - 2);
    str += "]";
    if(pr_inline)
      pr.print_inline(str);
    else
      pr.print(str);
  }

  void print(std::ostream &os) const {
    NbtPrinter pr(os);
    print(pr);
  }
};

typedef TagArray<std::int8_t, TAG_BYTE_ARRAY, "TagByteAray"> TagByteArray;
typedef TagArray<std::int16_t, TAG_INT_ARRAY, "TagIntArray"> TagIntArray;
typedef TagArray<std::int64_t, TAG_LONG_ARRAY, "TagLongArray"> TagLongArray;

inline void write_string(std::ostream &buf, const std::string &str) {
  std::uint16_t len = nbeswap(static_cast<std::uint16_t>(str.size()));
  buf.write(reinterpret_cast<char *>(&len), sizeof(len));
  buf.write(str.data(), str.size());
}

inline std::string read_string(std::istream &buf) {
    std::uint16_t len;
    buf.read(reinterpret_cast<char *>(&len), sizeof(len));
    len = nbeswap(len);
    auto tmp = std::make_unique<char[]>(len);
    buf.read(tmp.get(), len);
    return std::string(tmp.get(), len);
}

class TagString : public Tag<std::string, TAG_STRING> {
public:
  TagString() : Tag("TagString") {}
  TagString(std::string name) : Tag(name, "TagString") {}
  TagString(std::string val, std::string name) : Tag(val, name, "TagString") {}
  TagString(std::istream &buf) : Tag("TagString") {
    this->decode(buf);
  }
  TagString(std::istream &buf, std::string name) : Tag(name, "TagString") {
    this->decode(buf);
  }

  void decode(std::istream &buf) {
    this->val = read_string(buf);
  }

  void encode(std::ostream &buf) const {
    write_string(buf, this->val);
  }

  void print(NbtPrinter &pr, int pr_inline = 0) const {
    std::string str;
    if(this->name.has_value())
      str = this->type_name + "('" + this->name.value() + "'): '" +
      this->val + "'";
    else
      str = this->type_name + "(None): '" + this->val + "'";
    if(pr_inline)
      pr.print_inline(str);
    else
      pr.print(str);
  }
};

class TagList : public Tag<std::vector<std::unique_ptr<BaseTag>>, TAG_LIST> {
public:
  uint8_t list_id;
  TagList() : Tag("TagList") {}
  TagList(std::string name) : Tag(name, "TagList") {}
  TagList(std::istream &buf) : Tag("TagList") {
    this->decode(buf);
  }
  TagList(std::istream &buf, std::string name) : Tag(name, "TagList") {
    this->decode(buf);
  }

  void decode(std::istream &buf);
  void encode(std::ostream &buf) const;

  void print(NbtPrinter &pr, int pr_inline = 0) const {
    auto str = std::string(this->type_name + "('" + this->name.value_or("") +
    "'): " + std::to_string(this->val.size()) +
    ((this->val.size() == 1) ? " entry " : " entries ") + "{");
    if(pr_inline) {
      pr.print_inline(str);
      pr.print("");
      pr.inc_indent(2);
    }
    else
      pr.print(str);
    for(auto &el : this->val) {
      el->print(pr, 1);
      pr.print("");
    }
    if(pr_inline) {
      pr.dec_indent(2);
      pr.print_inline("}");
    }
    else
      pr.print("}");
  }
};

class TagCompound : public Tag
<std::unordered_map<std::string, std::unique_ptr<BaseTag>>, TAG_COMPOUND> {
public:
  TagCompound() : Tag("TagCompound") {}
  TagCompound(std::string name) : Tag(name, "TagCompound") {}
  TagCompound(std::istream &buf) : Tag("TagCompound") {
    this->decode(buf);
  }
  TagCompound(std::istream &buf, std::string name) : Tag(name, "TagCompound") {
    this->decode(buf);
  }

  void decode(std::istream &buf) {
    this->val.clear();
    std::uint8_t tag_type;
    buf.read(reinterpret_cast<char *>(&tag_type), sizeof(tag_type));
    while(tag_type != TAG_END) {
      std::string name = read_string(buf);
      switch (tag_type) {
        case TAG_BYTE:
          this->val.insert({name, std::make_unique<TagByte>(buf, name)});
          break;
        case TAG_SHORT:
          this->val.insert({name, std::make_unique<TagShort>(buf, name)});
          break;
        case TAG_INT:
          this->val.insert({name, std::make_unique<TagInt>(buf, name)});
          break;
        case TAG_LONG:
          this->val.insert({name, std::make_unique<TagLong>(buf, name)});
          break;
        case TAG_FLOAT:
          this->val.insert({name, std::make_unique<TagFloat>(buf, name)});
          break;
        case TAG_DOUBLE:
          this->val.insert({name, std::make_unique<TagDouble>(buf, name)});
          break;
        case TAG_BYTE_ARRAY:
          this->val.insert({name, std::make_unique<TagByteArray>(buf, name)});
          break;
        case TAG_STRING:
          this->val.insert({name, std::make_unique<TagString>(buf, name)});
          break;
        case TAG_LIST:
          this->val.insert({name, std::make_unique<TagList>(buf, name)});
          break;
        case TAG_COMPOUND:
          this->val.insert({name, std::make_unique<TagCompound>(buf, name)});
          break;
        case TAG_INT_ARRAY:
          this->val.insert({name, std::make_unique<TagIntArray>(buf, name)});
          break;
        case TAG_LONG_ARRAY:
          this->val.insert({name, std::make_unique<TagLongArray>(buf, name)});
          break;
      }
      buf.read(reinterpret_cast<char *>(&tag_type), sizeof(tag_type));
    }
  }

  void decode_full(std::istream &buf) {
    assert(buf.get() == TAG_COMPOUND);
    this->name = read_string(buf);
    decode(buf);
  }

  void encode(std::ostream &buf) const {
    for(auto &el : this->val) {
      buf.put(el.second->tag_id);
      write_string(buf, el.second->name.value());
      el.second->encode(buf);
    }
    buf.put(TAG_END);
  }

  void encode_full(std::ostream &buf) const {
    buf.put(this->tag_id);
    write_string(buf, this->name.value());
    encode(buf);
  }

  void print(NbtPrinter &pr, int pr_inline = 0) const {
    auto str = std::string(this->type_name + "('" + this->name.value_or("") +
    "'): " + std::to_string(this->val.size()) +
    ((this->val.size() == 1) ? " entry " : " entries ") + "{");
    if(pr_inline) {
      pr.print_inline(str);
      pr.print("");
      pr.inc_indent(2);
    }
    else
      pr.print(str);
    for(auto &el : this->val) {
      el.second->print(pr, 1);
      pr.print("");
    }
    if(pr_inline) {
      pr.dec_indent(2);
      pr.print_inline("}");
    }
    else
      pr.print("}");
  }
};

inline TagCompound read_compound(std::istream &buf) {
  std::uint8_t id;
  buf.read(reinterpret_cast<char *>(&id), sizeof(id));
  assert(id == TAG_COMPOUND);
  return TagCompound(buf, read_string(buf));
}

inline void write_compound(std::ostream &buf, TagCompound &tc) {
  tc.encode_full(buf);
}

void TagList::decode(std::istream &buf) {
  buf.read(reinterpret_cast<char *>(&list_id), sizeof(list_id));
  std::int32_t len;
  buf.read(reinterpret_cast<char *>(&len), sizeof(len));
  len = nbeswap(len);
  if(len <= 0)
    return;
  this->val.reserve(len);
  switch (list_id) {
    case TAG_BYTE:
      for(int i = 0; i < len; i++)
        this->val.push_back(std::make_unique<TagByte>(buf));
      break;
    case TAG_SHORT:
      for(int i = 0; i < len; i++)
        this->val.push_back(std::make_unique<TagShort>(buf));
      break;
    case TAG_INT:
      for(int i = 0; i < len; i++)
        this->val.push_back(std::make_unique<TagInt>(buf));
      break;
    case TAG_LONG:
      for(int i = 0; i < len; i++)
        this->val.push_back(std::make_unique<TagLong>(buf));
      break;
    case TAG_FLOAT:
      for(int i = 0; i < len; i++)
        this->val.push_back(std::make_unique<TagFloat>(buf));
      break;
    case TAG_DOUBLE:
      for(int i = 0; i < len; i++)
        this->val.push_back(std::make_unique<TagDouble>(buf));
      break;
    case TAG_BYTE_ARRAY:
      for(int i = 0; i < len; i++)
        this->val.push_back(std::make_unique<TagByteArray>(buf));
      break;
    case TAG_STRING:
      for(int i = 0; i < len; i++)
        this->val.push_back(std::make_unique<TagString>(buf));
      break;
    case TAG_LIST:
      for(int i = 0; i < len; i++)
        this->val.push_back(std::make_unique<TagList>(buf));
      break;
    case TAG_COMPOUND:
      for(int i = 0; i < len; i++)
        this->val.push_back(std::make_unique<TagCompound>(buf));
      break;
    case TAG_INT_ARRAY:
      for(int i = 0; i < len; i++)
        this->val.push_back(std::make_unique<TagIntArray>(buf));
      break;
    case TAG_LONG_ARRAY:
      for(int i = 0; i < len; i++)
        this->val.push_back(std::make_unique<TagLongArray>(buf));
      break;
  }
}

void TagList::encode(std::ostream &buf) const {
  buf.put(list_id);
  std::int32_t len = nbeswap(static_cast<std::int32_t>(this->val.size()));
  buf.write(reinterpret_cast<char *>(&len), sizeof(len));
  for(auto &el : this->val)
    el->encode(buf);
}

} // namespace NBT
#endif // NBT_HPP
