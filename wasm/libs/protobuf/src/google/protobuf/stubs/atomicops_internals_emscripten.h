// This file is an internal atomic implementation, use atomicops.h instead.

#ifndef GOOGLE_PROTOBUF_ATOMICOPS_INTERNALS_EMSCRIPTEN_H_
#define GOOGLE_PROTOBUF_ATOMICOPS_INTERNALS_EMSCRIPTEN_H_

namespace google {
    namespace protobuf {
        namespace internal {
            
            // 32-bit low-level operations on any platform.
            
            inline Atomic32 NoBarrier_CompareAndSwap(volatile Atomic32* ptr,
                                                                                               Atomic32 old_value,
                                                                                               Atomic32 new_value) {
                  Atomic32 old_reg = *ptr;
                  if (old_reg == old_value)
                    *ptr = new_value;
                
                  return old_reg;
                }
            
            inline Atomic32 NoBarrier_AtomicExchange(volatile Atomic32* ptr,
                                                                                               Atomic32 new_value) {
                  Atomic32 prev = *ptr;
                  *ptr = new_value;
                  return prev;
                }
            
            inline Atomic32 NoBarrier_AtomicIncrement(volatile Atomic32* ptr,
                                                                                                 Atomic32 increment) {
                  Atomic32 prev = *ptr;
                  *ptr = *ptr + increment;
                  return prev;
                }
            
            inline Atomic32 Barrier_AtomicIncrement(volatile Atomic32* ptr,
                                                                                             Atomic32 increment) {
                 Atomic32 prev = *ptr;
                  *ptr = *ptr + increment;
                  return prev;
                }
            
            inline Atomic32 Acquire_CompareAndSwap(volatile Atomic32* ptr,
                                                                                           Atomic32 old_value,
                                                                                           Atomic32 new_value) {
                  Atomic32 old_reg = *ptr;
                  if (old_reg == old_value)
                    *ptr = new_value;
                
                  return old_reg;
                }
            
            inline Atomic32 Release_CompareAndSwap(volatile Atomic32* ptr,
                                                                                           Atomic32 old_value,
                                                                                           Atomic32 new_value) {
              Atomic32 old_reg = *ptr;
              if (old_reg == old_value)
                *ptr = new_value;
            
              return old_reg;
            }
            
            inline void NoBarrier_Store(volatile Atomic32* ptr, Atomic32 value) {
                  *ptr = value;
                }
            
            inline void MemoryBarrier() {
                
                }
            
            inline void Acquire_Store(volatile Atomic32* ptr, Atomic32 value) {
                    *ptr = value;
                }
            
            inline void Release_Store(volatile Atomic32* ptr, Atomic32 value) {
                  *ptr = value;
                }
            
            inline Atomic32 NoBarrier_Load(volatile const Atomic32* ptr) {
                  return *ptr;
                }
            
            inline Atomic32 Acquire_Load(volatile const Atomic32* ptr) {
                  Atomic32 value = *ptr;
                  return value;
                }
            
            inline Atomic32 Release_Load(volatile const Atomic32* ptr) {
                  return *ptr;
                }
            
            }  // namespace internal
        }  // namespace protobuf
    }  // namespace google


#endif // GOOGLE_PROTOBUF_ATOMICOPS_INTERNALS_EMSCRIPTEN_H_
