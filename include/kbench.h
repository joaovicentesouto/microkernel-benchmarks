/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _KBENCH_H_
#define _KBENCH_H_

	#include <nanvix/runtime/stdikc.h>
	#include <nanvix/sys/dev.h>
	#include <stdint.h>

	/**
	 * @brief Number of benchmark iterations.
	 */
	#ifdef NDEBUG
		#define NITERATIONS 1
	#else
		#define NITERATIONS 1
	#endif

	/**
	 * @brief Iterations to skip on warmup.
	 */
	#define SKIP 10

	/**
	 * @brief Casts something to a uint32_t.
	 */
	#define UINT32(x) ((uint32_t)((x) & 0xffffffff))

	/**
	 * @brief An horizontal line.
	 */
	#define HLINE "--------------------------------------------------------------------------------\n"

	/**
	 * @brief Generic fence.
	 */
	int fence_setup(int nioclusters, int ncclusters);
	int fence_cleanup(void);
	int fence(void);

	/**
	 * Build a list of the node ids.
	 */
	void build_node_list(int nioclusters, int ncclusters, int *nodes);

	/**
	 * Waits a number of cicles.
	 */
	void timer(uint64_t cicles);

/*============================================================================*
 * Convert Functions                                                          *
 *============================================================================*/

	/**
	 *  Convert a string to a int.
	 */
	int __atoi(const char * str);

	/**
	 *  Convert a int to a string.
	 */
	char * __itoa(int num, char * str, int base);

/*============================================================================*
 * Memory Functions                                                           *
 *============================================================================*/

	/**
	 * @brief Fills words in memory.
	 *
	 * @param ptr Pointer to target memory area.
	 * @param c   Character to use.
	 * @param n   Number of bytes to be set.
	 */
	static inline void memfill(word_t *ptr, word_t c, size_t n)
	{
		word_t *p;

		p = ptr;

		/* Set words. */
		for (size_t i = 0; i < n; i++)
			*p++ = c;
	}

	/**
	 * @brief Copy words in memory.
	 *
	 * @param dest Target memory area.
	 * @param src  Source memory area.
	 * @param n    Number of bytes to be copied.
	 */
	static inline void memcopy(word_t *dest, const word_t *src, size_t n)
	{
		word_t *d;       /* Write pointer. */
		const word_t* s; /* Read pointer.  */

		s = src;
		d = dest;

		/* Copy words. */
		for (size_t i = 0; i < n; i++)
			*d++ = *s++;
	}

#endif /* _KBENCH_H_ */
