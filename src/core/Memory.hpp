#pragma region Copyright (c) 2014-2016 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#pragma once

#include "../common.h"
#include "Guard.hpp"

/**
 * Utility methods for memory management. Typically helpers and wrappers around the C standard library.
 */
namespace Memory
{
    template<typename T>
    T * Allocate()
    {
        return (T*)malloc(sizeof(T));
    }

    template<typename T>
    T * Allocate(size_t size)
    {
        return (T*)malloc(size);
    }

    template<typename T>
    T * AllocateArray(size_t count)
    {
        return (T*)malloc(count * sizeof(T));
    }

    template<typename T>
    T * Reallocate(T * ptr, size_t size)
    {
        if (ptr == nullptr)
        {
            return (T*)malloc(size);
        }
        else
        {
            return (T*)realloc((void*)ptr, size);
        }
    }

    template<typename T>
    T * ReallocateArray(T * ptr, size_t count)
    {
        if (ptr == nullptr)
        {
            return (T*)malloc(count * sizeof(T));
        }
        else
        {
            return (T*)realloc((void*)ptr, count * sizeof(T));
        }
    }

    template<typename T>
    void Free(T * ptr)
    {
        free((void*)ptr);
    }

    template<typename T>
    T * Copy(T * dst, const T * src, size_t size)
    {
        if (size == 0) return (T*)dst;
#ifdef DEBUG
        uintptr_t srcBegin = (uintptr_t)src;
        uintptr_t srcEnd = srcBegin + size;
        uintptr_t dstBegin = (uintptr_t)dst;
        uintptr_t dstEnd = dstBegin + size;
        Guard::Assert(srcEnd <= dstBegin || srcBegin >= dstEnd, "Source overlaps destination, try using Memory::Move.");
#endif
        return (T*)memcpy((void*)dst, (const void*)src, size);
    }

    template<typename T>
    T * Move(T * dst, const T * src, size_t size)
    {
        if (size == 0) return (T*)dst;
        return (T*)memmove((void*)dst, (const void*)src, size);
    }

    template<typename T>
    T * Duplicate(const T * src, size_t size)
    {
        T *result = Allocate<T>(size);
        return Copy(result, src, size);
    }

    template<typename T>
    T * Set(T * dst, uint8 value, size_t size)
    {
        return (T*)memset((void*)dst, (int)value, size);
    }

    template<typename T>
    T * CopyArray(T * dst, const T * src, size_t count)
    {
        // Use a loop so that copy constructors are called
        // compiler should optimise to memcpy if possible
        T * result = dst;
        for (; count > 0; count--)
        {
            *dst++ = *src++;
        }
        return result;
    }

    template<typename T>
    T * DuplicateArray(const T * src, size_t count)
    {
        T * result = AllocateArray<T>(count);
        return CopyArray(result, src, count);
    }

    template<typename T>
    void FreeArray(T * ptr, size_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            ptr[i].~T();
        }
        Free(ptr);
    }

	template<typename T>
	T * AllocateAligned(size_t alignment, size_t size)
	{
		size_t remainder = size % alignment;
		size_t zero_pad = (remainder > 0) ? (alignment - remainder) : 0;

		// Allocate enough space for ...:
		//  - the alignment
		//  - a pointer to the original return value (needed for free)
		//  - the object
		//  - extra zero-padded space to bring object size to a multiple of alignment
		//  - extra space for alignment
		uint8 *ptr = (uint8 *)malloc(sizeof(size_t) + sizeof(uint8 *) + size + alignment - 1 + zero_pad);
		if (ptr == NULL) {
			log_error("Failed to allocate memory for object.");
			assert(false);
		}

		// Align chunk pointer on boundary.
		uint8 *ptr_aligned = (uint8 *)((uint8 *)ptr + sizeof(size_t) + sizeof(uint8 *) + alignment - 1 - remainder);

		// Save alignment for realloc
		((size_t *)((uint8 *)ptr_aligned - sizeof(uint8 *)))[-1] = alignment;
		// Save original pointer for free
		((uint8 **)ptr_aligned)[-1] = ptr;

		// Zero-pad object up to a multiple of alignment
		memset((uint8 *)ptr_aligned + size, 0x00, zero_pad);

		return (T*)ptr_aligned;
	}

	template<typename T>
	T * AllocateAligned(size_t alignment)
	{
		return AllocateAligned<T>(alignment, sizeof(T));
	}

	template<typename T>
	void FreeAligned(T * ptr)
	{
		free((void *)(((uint8 **)ptr)[-1]));
	}

	template<typename T>
	T * ReallocateAligned(T * ptr_aligned, size_t size)
	{
		uint8 *ptr_new, *ptr_new_aligned;
		uint8 *ptr_old = ((uint8 **)ptr_aligned)[-1];

		size_t alignment = ((size_t *)ptr_aligned)[-2];
		size_t remainder = size % alignment;
		size_t zero_pad = (remainder > 0) ? (alignment - remainder) : 0;
		size_t offset = (size_t)((uint8 *)ptr_aligned - (uint8 *)ptr_old);

		ptr_new = (uint8 *)realloc(ptr_old, offset + size + zero_pad);
		if (ptr_new == NULL) {
			log_error("Failed to reallocate memory for object.");
			assert(false);
		}

		ptr_new_aligned = (uint8 *)((uint8 *)ptr_new + offset);
		((uint8 **)ptr_new_aligned)[-1] = ptr_new;
		memset((uint8 *)ptr_new_aligned + size, 0x00, zero_pad);

		// realloc moved the data (fine) to a non-aligned location (not fine)
		if ((uint32)ptr_new_aligned % alignment > 0) {
			uint8 *tmp = AllocateAligned<T>(alignment, size);

			memcpy(tmp, ptr_new_aligned, size);
			FreeAligned<T>(ptr_new_aligned);

			ptr_new_aligned = tmp;
		}

		return (T*)ptr_new_aligned;
	}
}
