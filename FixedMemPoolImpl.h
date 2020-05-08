#ifndef _FIXED_MEM_POOL_IMPL_H_
#define _FIXED_MEM_POOL_IMPL_H_

/*
  This template class contains the definitions of the methods of
  fixed-size memory pool.
*/

#include <cassert>
#include <iostream>

#include "FixedMemPool.h"

using namespace std;

template <typename T>
FixedSizeMemoryPool<T>::FixedSizeMemoryPool()
: m_pool_(static_cast<MemoryCb*>(
      ::operator new(sizeof(MemoryCb) * g_MaxNumberOfObjectsInPool))),
  free_head_(&m_pool_[0]),
  free_count_(g_MaxNumberOfObjectsInPool),
  fresh_index_(0) {
}

template <typename T>
FixedSizeMemoryPool<T>::~FixedSizeMemoryPool() {
    assert(m_pool_ != nullptr);

    try {
        // Avoid calling destructor of the caller class.
      ::operator delete(m_pool_);
    } catch(...) {
      assert(false);
    }

    m_pool_ = nullptr;
    free_head_ = nullptr;
}

template <typename T>
template <typename ...Args>
T* FixedSizeMemoryPool<T>::Allocate(Args ...args) {
    assert(free_count_ != 0);
    assert(m_pool_ != nullptr);

    // Check if there is any memory block that has never been allocated.
    // If yes, then set its next_free_index to the next memory block.
    // The idea here is that, the next_free_index builds a notion of free-list
    // inside the system. This field is never set for the first time and hence,
    // it should be explictly set.
    // After all the memory block's next_free_index block have been set, the
    // control will never come to this if-condition. This is because on each
    // allocation fresh_index value will increase.
    // This helps in achieving O(1) time complexity at pool creation and
    // allocation.
    if (fresh_index_ < g_MaxNumberOfObjectsInPool) {
        MemoryCb* cb = ToAddr(fresh_index_);
        cb->next_free_index_ = ++fresh_index_;
    }

    // The free_head points to the free memory block.
    // Retrieve the data_ field, which is the memory that will be returned
    // to the caller.
    T* mem = &free_head_->data_;

    // Point the free_head to the next free memory block in the system.
    free_head_ = ToAddr(free_head_->next_free_index_);
    --free_count_;

    try {
        // Call the constructor of the class with the provided arguments.
        new(mem) T(std::forward<Args>(args)...);
    } catch(...) {
        assert(false);
    }

    return mem;
}

template <typename T>
void FixedSizeMemoryPool<T>::Deallocate(T* ptr) {
    if (ptr == nullptr) return;

    // Typecast the caller memory to get the memory control block.
    MemoryCb *cb = reinterpret_cast<MemoryCb*>(ptr);

    // Check if this memory is owned by the pool. If not an assert failure will
    // happen.
    ValidateMemory(cb);

    // Call the destructor.
    ptr->~T();
    ++free_count_;

    if (free_head_ != nullptr) {
        // Get the current free memory block and set its next free memory block
        // index to that of head of free memory block.
        cb->next_free_index_ = ToIndex(free_head_);

        // Set current memory block as the head of the free memory block.
        free_head_ = cb;
    } else {
        // The free head is empty, set this memory block as the head.
        free_head_ = cb;
        cb->next_free_index_= g_MaxNumberOfObjectsInPool;
    }
}

template <typename T>
typename FixedSizeMemoryPool<T>::MemoryCb*
FixedSizeMemoryPool<T>::ToAddr(int index) {
    assert (index >= 0);
    if (index >= g_MaxNumberOfObjectsInPool) return nullptr;
    return &m_pool_[index];
}

template <typename T>
int FixedSizeMemoryPool<T>::ToIndex(MemoryCb* ptr) {
    assert (ptr != nullptr);
    return (ptr - m_pool_);
}

template <typename T>
void FixedSizeMemoryPool<T>::ValidateMemory(MemoryCb* cb) {
    // Find the start and end memory block address and check if the current
    // memory block falls within the address range.
    uintptr_t cb_addr = reinterpret_cast<uintptr_t>(cb);
    uintptr_t start_addr = reinterpret_cast<uintptr_t>(&m_pool_[0]);
    uintptr_t end_addr = reinterpret_cast<uintptr_t>(
      &m_pool_[g_MaxNumberOfObjectsInPool - 1]);

    assert (cb_addr >= start_addr && cb_addr <= end_addr);
}

#ifdef UNIT_TEST

template <typename T>
const uint64_t FixedSizeMemoryPool<T>::Capacity() const {
    return g_MaxNumberOfObjectsInPool;
}

template <typename T>
const uint64_t FixedSizeMemoryPool<T>::FreeSize() const {
    return free_count_;
}

template <typename T>
const typename FixedSizeMemoryPool<T>::AddressList
FixedSizeMemoryPool<T>::GetAddressList() const {
    AddressList addr_set;

    // Iterate through the list of memory blocks and prepare a set of
    // addresses that could be returned to the caller when allocate
    // method is called.
    for (int i = 0; i < g_MaxNumberOfObjectsInPool; i++) {
        addr_set.emplace(
            reinterpret_cast<uintptr_t>(&(m_pool_[i].data_)));
    }

    return addr_set;
}
#endif

#endif
