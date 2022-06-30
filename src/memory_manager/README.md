# Design of Alloctor

## Map of sub-pages

- 1 wasm-page is 64k, divided 16 sub-page, each sub-page 4k
- sub-page need 1 bit-flag, for is used
- need 16 bits to store map in the beginning of wasm-page

## Layout of sub-page

- Double Linked List

  1. 32B
  2. 64B
  3. 128B
  4. 256B
  5. 512B
  6. 1024B
  7. \>2048B

- buddy allocator

## Port to Assemblyscript

How to handle that global and shadow stack cause heap_pointer is not aligned
