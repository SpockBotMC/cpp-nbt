# cpp-nbt

This is a C++20 header-only library for reading/writing
[Minecraft NBT](https://wiki.vg/NBT) data:
* Single header file
* Requires C++20 (GCC 10.1+, Clang trunk, VS 16.8.2+)
* Reads from `istream`s, writes to `ostream`s
* Supports pretty printing

## Quickstart

Include nbt.hpp, munch some data.

```C++
#include "nbt.hpp"

std::ifstream ifs("hello_world.nbt");
auto tc = nbt::read_compound(ifs);
std::cout << tc;
```
## Tags

All PC-edition NBT tags are supported, Bedrock is not currently supported.

All tags have a `.val` member that maps to a STL container or fundamental type that can be easily manipulated. All tags inherit from `BaseTag`, which can be used to identify the derived tag.

### Constructing and Destructing Tags

The most common method of interacting with NBT is to encode or decode a
containing `TagCompound`. **cpp-nbt** supports two methods for this. This first
is to use the convenience methods `TagCompound read_compound(std::istream&)` and
`void write_compound(std::ostream&, TagCompound&)`.

Example Usage:
```C++
std::ifstream ifs("hello_world.nbt");
auto tc = nbt::read_compound(ifs);

std::ofstream ofs("out.nbt");
nbt::write_compound(ofs, tc);
```

You can also use the `TagCompound` methods `.decode_full(std::istream&)` and
`.encode_full(std::ostream&)`.

Example Usage:
```C++
nbt::TagCompound tc;

std::ifstream ifs("hello_world.nbt");
tc.decode_full(ifs);

std::ofstream ofs("out.nbt");
tc.encode_full(ofs);
```

### BaseTag
`const std::uint8_t type_id` : The [Type ID](https://wiki.vg/NBT#Specification) of the tag, Type IDs are enum'd in all upper case

Example Usage:
```C++
void f(BaseTag &tag) {
  switch(tag.tag_id) {
    case TAG_FLOAT:
      std::cout << "Float is: " << dynamic_cast<nbt::TagFloat&>(tag).val;
      break;
    case TAG_LONG:
      std::cout << "Long is: " << dynamic_cast<nbt::TagLong&>(tag).val;
      break;
    }
}
```

`const std::string type_name`: String of the Type ID

Example Usage:
```C++
nbt::TagDouble x;
std::cout << x.type_name;
```
Outputs: "TagDouble"

`std::optional<std::string> name`: Name of the Tag, has no value for nameless
tags such as those contained inside `TagList`s

### Derived Tags `.val`s

* `TagByte` -> `std::int8_t`
* `TagShort` -> `std::int16_t`
* `TagInt`   -> `std::int32_t`
* `TagLong` -> `std::int64_t`
* `TagFloat` -> `float`
* `TagDouble` -> `double`
* `TagByteArray` -> `std::vector<std::int8_t>`
* `TagIntArray` -> `std::vector<std::int32_t>`
* `TagLongArray` -> `std::vector<std::int64_t>`
* `TagList` -> `std::vector<std::unique_ptr<BaseTag>>`
* `TagCompound` -> `std::unordered_map<std::string, std::unique_ptr<BaseTag>>`

### Caveats

* All the elements of a `TagList` should point to the same type of Tag. **cpp-nbt** will not stop you from encoding a malformed list.

* The keys of `TagCompound` have no effect on how it is encoded. The tags
inside the `TagCompound` encode their names based on their `.name` member, not
the key.

## Issues

Please open an issue or better yet, a pull request, for any bugs or other
problems.
