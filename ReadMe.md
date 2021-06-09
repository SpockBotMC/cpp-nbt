# cpp-nbt

![build](https://github.com/SpockBotMC/cpp-nbt/actions/workflows/main.yml/badge.svg)

This is a C++20 header-only library for reading/writing
[Minecraft NBT](https://wiki.vg/NBT) data:
* Single header file
* Requires C++20 (GCC 11, Clang 12, VS 16.9)
* Reads from `istream`s, writes to `ostream`s
* Supports pretty printing

## Quickstart

Include nbt.hpp, munch some data.

```cpp
#include "nbt.hpp"

nbt::NBT root {std::ifstream {"hello_world.nbt"}};
std::cout << root;
```

## Tags

All PC-edition NBT tags are supported, Bedrock is not currently supported.

Most tags map directly to primitive types, the remainder map to STL containers.

| Tag Type | Description |
| --- | --- |
| `TagEnd` | `typedef std::nullptr_t` |
| `TagByte` | `typedef std::int8_t` |
| `TagShort` | `typedef std::int16_t` |
| `TagInt` | `typedef std::int32_t` |
| `TagLong` | `typdef std::int64_t` |
| `TagFloat` | `typedef float` |
| `TagDouble` | `typedef double` |
| `TagByteArray` | `typedef std::vector<TagByte>` |
| `TagIntArray` | `typedef std::vector<TagInt>` |
| `TagLongArray` | `typedef std::vector<TagLong>` |
| `TagString` | `typedef std::string` |
| `TagList` | Thin wrapper around `std::variant<[vectors of all tags]>` |
| `Tag` | `std::variant<[all tags]>` |
| `TagCompound` | Thin wrapper around `std::map<std::string, Tag>`|
| `NBT` | A named `TagCompound` that can .encode/decode binary data |

The underlying variant and map of `TagList` and `TagCompound` can be access
directly using their `.base` member variable. Some operators and constructors
have been mirrored for convenience, but if you need anything fancy from the STL
just use the member variable directly.


### Constructing and Destructing Tags

Java edition NBT root nodes are always either a TagEnd, or a named TagCompound.
These two root nodes are encapsulated by the `NBT` type, which has an
`encode()` and `decode()` methods to serialize and deserialize NBT.

Example Usage:
```cpp
std::ifstream ifs {"hello_world.nbt"};
nbt::NBT root;
root.decode(ifs);
// Optionally, use the constructor
// nbt::NBT root {ifs};


std::ofstream ofs {"out.nbt"};
root.encode(ofs);
```

### Manipulating Tags

Tags must exist inside a container type, and manipulating NBT containers
involves standard usage of the STL.

```cpp
nbt::NBT root {"LyricalNBT", {
  {"Hello", nbt::TagString {"World"}},
  {"Lyrics", nbt::TagList {
    std::vector<nbt::TagString> {
      "There's", "a", "song", "that", "we're", "singing",
    },
  },},
}};

root["LuckyNumbers"] = nbt::TagByteArray {1, 3, 7, 9, 13, 15};
root.at<nbt::TagByteArray>("LuckyNumbers").push_back(21);

std::cout << root.at<nbt::TagString>("Hello") << std::endl;

std::cout << root << std::endl;
```

A convenience method, `get_list<>` is available for extracting the vector out
of a TagList.

```cpp
auto& tag {root.at<nbt::TagList>("lyrics")};
std::vector<nbt::TagString>& lyrics {nbt::get_list<TagString>(tag)};

for(const std::string& word : lyrics)
  std::cout << word << " ";
```


## Issues

Please open an issue or better yet, a pull request, for any bugs or other
problems.
