---- FIXED SIZE MEMORY POOLER ----

1. Build:
   The code has been build using g++ version 5.4.0 on Ubuntu-16.04 using C++14

2. Time complexity:
   Create pool - O(1)
   Destroy pool - O(1)
   Allocate - O(1)
   Deallocate O(1)

3. Usage:
  Consider pooling a class called Base which takes integer in the constructor.

  - Create the object of this class.
    FixedSizeMemoryPool<Base> pool;

  - Allocate a memory with integer 10 passed to the constructor.
    int* mem = pool.Allocate(10);

  - Deallocate the memory.
    pool.Deallocate();

   - Pool memory is destroy when pooler destructor is called.
