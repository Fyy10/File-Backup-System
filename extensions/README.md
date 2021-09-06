# Extensions

  - [ToDo](#todo)
  - [listen_file_change](#listen_file_change)

## ToDo

- Encode & Decode (Huffman)

## listen_file_change

Listen changes (create, delete, modify, etc.) on directories or files.

```cpp
#include "listen_file_change.hpp"
// create a watcher object and listen events
Watcher watcher("./file_to_listen");
watcher.handle_events();
```