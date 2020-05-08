#ifndef _FIXED_MEM_POOL_H_
#define _FIXED_MEM_POOL_H_

/*
  This template class implements a fixed-size memory pool.
  The class is tightly bound to the type of the class that wants to allocate
  memory from the pool.
  The size of the pool is governed by the global variable
  g_MaxNumberOfObjectsInPool.

  The pool system at the time of memory allocation will automatically call the
  constructor of the class.

  The pool system at the time of deallocation will automatically call the
  destructor of the class.


  Usage:
  Consider pooling integer object.

  - Create the object of this class.
    FixedSizeMemoryPool<int> pool;

  - Allocate a memory
    int* mem = pool.Allocate();

  - Deallocate the memory.
    pool.Deallocate();
*/

#include <cinttypes>

#ifdef UNIT_TEST
#include <set>
#endif

// Set the maximum numbers of memory blocks to be pooled using this variable.
const int g_MaxNumberOfObjectsInPool = 1000;

template <typename T>
class FixedSizeMemoryPool {
public:

#ifdef UNIT_TEST
    // Typedefed set, which holds the list of the address in the pool.
    // This is mainly used for validating addresses while unit-testing.
    typedef std::set <uintptr_t> AddressList;
#endif

    // Memory control block used by pooler to manage memory.
    struct MemoryCb {
        // Initialize next_free_index with invalid index number.
        MemoryCb(): next_free_index_(g_MaxNumberOfObjectsInPool) {}

        // Pointer of this variable will be returned to the caller.
        T data_;

        // A variable to keep track on next free index in the array of memory.
        uint64_t next_free_index_;
    };

    // Constructor allocates the memory required to pool the objects.
    // The running time to create pool is O(1).
    FixedSizeMemoryPool();

    // Destructor destroys the memory that is being used for pooling the
    // objects.
    // The runing time to destroy pool is O(1).
    virtual ~FixedSizeMemoryPool();

    // Caller calls this method to allocate the memory from the pool.
    // The method is templatized to consider for user provided arguments in
    // the constructor.
    // This method internally calls the constructor of the class.
    // The running time of this method is O(1).
    template <typename ...Args>
     T* Allocate(Args ...args);

     // Caller calls this method to deallocate the memory from the pool.
     // The running time of this method is O(1).
     void Deallocate(T* ptr);

#ifdef UNIT_TEST
     // This method gives the count of maximum number of objects that can be
     // pooled.
     const uint64_t Capacity() const;

     // This method gives the count of free memory blocks in the system.
     const uint64_t FreeSize() const;

     // This method gives the full list of addresses that will be returned to
     // the caller on calling Allocate method.
     const AddressList GetAddressList() const;
#endif

private:
    // A wrapper method to get the address of the memory block from its index.
    MemoryCb* ToAddr(int index);

    // A wrapper method to get index of the memory block from its address.
    int ToIndex(MemoryCb* ptr);

    // A method to validate if the memory returned by the caller upon
    // deallocation is a memory owned by the pooler. An assertion check is
    // made against that.
    void ValidateMemory(MemoryCb *cb);

private:
    // A pointer to hold the contigous array of memory control blocks.
    MemoryCb* m_pool_;

    // A pointer pointing to the head of the free control block.
    MemoryCb* free_head_;

    // A variable to keep track of number of free memory blocks available.
    uint64_t free_count_;

    // A variable holding the index of the next never used control block.
    uint64_t fresh_index_;
};

#endif
