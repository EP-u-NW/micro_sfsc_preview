/*
 * SFSC_Platform.h
 *
 *  Created on: 30.06.2020
 *      Author: Eric
 */

#ifndef SRC_SFSC_PLATFORM_H_
#define SRC_SFSC_PLATFORM_H_

#include "sfsc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generates random bytes and writes them into a buffer.
 *
 * This function is not used for cryptographic operations, so the
 * generated sequence can be cryptograpically weak (e.g. pseudorandom).
 * It is important doe, that the generated sequences are different
 * after each system start!
 *
 * @param buffer the buffer the bytes should be written to
 * @param size the amount of bytes to generate
 */
void random_bytes(sfsc_uint8* buffer, sfsc_size size);

/**
 * @brief Gives some measurement of time in milliseconds.
 *
 * The time should be given in a millisecond resulution.
 * It is not neccessary (but allowed) to give the unix time here.
 * It is perfectly fine to return the time since system start.
 *
 * @return sfsc_uint64 The time in milliseconds
 */
sfsc_uint64 time_ms();

/**
 * @brief Locks a socket.
 *
 * The pointers address identifies the socket you should lock, while
 * the address it points to is a byte of memory you can  freely use,
 * e.g. to store locking information.
 *
 * Locking should work the follwing way: Once this function is called,
 * you should check from which thread the call was made. Then, if an
 * other thread called the lock function with the same pointer before,
 * but did not call the unlock function, the address is saied
 * to be locked, and the other thread holds that lock.
 * This function should then block, until the thread,
 * that currently holds the lock, releases it by calling the unlock
 * function. Lastly, the thread that called this function acquires
 * the lock, and this function should return. If this function is
 * called again by the thread that already holds the lock, it should
 * just return.
 *
 * This function only needs to be impelemented if you call framework functions
 * from different threads (including the system_task and user_task functions).
 * If you don't do this, you should provide a no-op-impelmentation of this
 * function (doing nothing in it).
 *
 * Synchronization can be a hard-to-implement concept, but so is
 * multi-threading. Chances are, that if you use a premade kernel/scheduler or
 * operating system, there are already techniques for synchronization you can
 * reuse here.
 *
 * @param lock_mem A pointer that uniquely identifies the socket to lock,
 * pointing to a memory area you can freely use
 */
void lock_socket(sfsc_uint8* lock_mem);
/**
 * @brief Unlocks a socket.
 * 
 * This is the contrary function to lock_socket. The locking process
 * is described in depth there.
 * 
 * @param lock_mem A pointer that uniquely identifies the socket to unlock,
 * pointing to a memory area you can freely use
 */
void unlock_socket(sfsc_uint8* lock_mem);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SRC_SFSC_PLATFORM_H_ */
