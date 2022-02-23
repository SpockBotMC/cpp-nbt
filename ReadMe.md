# cpp-nbt

[![license](https://img.shields.io/badge/license-zlib-lightgrey.svg)](https://en.wikipedia.org/wiki/Zlib_License)


This is a C++20 header-only library for reading/writing
[Minecraft NBT](https://wiki.vg/NBT) data:
* Single header file
* Requires C++20 and [P2162](https://wg21.link/p2162) (GCC 13, Clang 14, MSVC 19.latest)
* Reads from `istream`s, writes to `ostream`s
* Supports pretty printing

This is mostly for me to play with bleeding edge C++ stuff. Don't expect
this to compile on anything but trunk.

## Quickstart

`std::map` doesn't work with incomplete types portably, so you need to provide
your own (`std::map` may work depending on your platform and stdlib). By
default cpp-nbt uses `boost::container::map`. You can do this by defining
`NBT_MAP_TYPE` to whatever type you want to use, so long it has a vaguely
`std::map`-ish API.

Include nbt.hpp, munch some data.

```cpp
#define NBT_MAP_TYPE myFavoriteMap
#include "nbt.hpp"

nbt::NBT root {std::ifstream {"hello_world.nbt"}};
std::cout << root;
```

## Tags

All PC-edition NBT tags are supported, Bedrock is not currently supported.

Most tags map directly to primitive types, the remainder map to STL containers.

| Tag Type | Description |
| --- | --- |
| `TagEnd` | `std::nullptr_t` |
| `TagByte` | `std::int8_t` |
| `TagShort` | `std::int16_t` |
| `TagInt` | `std::int32_t` |
| `TagLong` | `std::int64_t` |
| `TagFloat` | `float` |
| `TagDouble` | `double` |
| `TagByteArray` | `std::vector<TagByte>` |
| `TagIntArray` | `std::vector<TagInt>` |
| `TagLongArray` | `std::vector<TagLong>` |
| `TagString` | `std::string` |
| `TagList` | `std::variant<std::vectors<TagEnd>, std::vector<TagByte>, and all other tags>` |
| `Tag` | `std::variant<TagEnd, TagByte, and all other tags>` |
| `TagCompound` | `std::map<TagString, Tag>`|
| `NBTData` | `struct { TagString name; TagCompound tags; }` |
| `NBT` | `struct {std::optional<NBTData> data; }` |


### Constructing and Destructing Tags

Java edition NBT root nodes are always either a TagEnd, or a named TagCompound.
These two root nodes are encapsulated by the `NBT` type, which has an
`encode()` and `decode()` methods to serialize and deserialize NBT. When
the TagCompound is present the `data` field will be present, otherwise it
will be absent. If absent, the NBT container will serialize to a single `TagEnd`.

Example Usage:
```cpp
nbt::NBT root;
root.decode(std::ifstream {"hello_world.nbt"});
// Optionally, use the constructor
// nbt::NBT root {ifs};

root.encode(std::ofstream {"out.nbt"});
```

### Manipulating Tags

Tags must exist inside a container type, and manipulating NBT containers
involves standard usage of the STL.

```cpp
nbt::NBT root {"LyricalNBT", {
  {"Hello", "World"},
  {"Lyrics", nbt::TagList {
      "There's", "a", "song", "that", "we're", "singing",
  }},
}};

root["LuckyNumbers"] = nbt::TagByteArray {1, 3, 7, 9, 13, 15};
std::get<nbt::TagByteArray>(root["LuckyNumbers"]).push_back(21);

std::cout << root["Hello"] << std::endl;
std::cout << root << std::endl;
```


## Issues

Please open an issue or better yet, a pull request, for any bugs or other
problems.
