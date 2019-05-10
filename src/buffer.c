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

#include <sys/types.h>
#include <nanvix.h>
#include <stdint.h>
#include "kbench.h"

#ifdef __benchmark_buffer__

/**
 * @name Benchmark Parameters
 */
/**@{*/
#define NTHREADS_MIN                2  /**< Minimum Number of Working Threads      */
#define NTHREADS_MAX  (THREAD_MAX - 1) /**< Maximum Number of Working Threads      */
#define NTHREADS_STEP               2  /**< Increment on Number of Working Threads */
#define OBJSIZE                  8192  /**< Object Size                            */
#define BUFLEN                      8  /**< Buffer Length                          */
#define NOBJECTS                   16  /**< Number of Objects                      */
/**@}*/

/**
 * @name Benchmark Kernel Parameters
 */
/**@{*/
static int NTHREADS;   /**< Number of Working Threads */
/**@}*/

/**
 * @brief Current benchmark iteration.
 */
static int iteration = 0;

/**
 * @brief Buffer.
 */
struct buffer
{
	size_t first;
	size_t last;
	struct nanvix_mutex mutex;
	struct nanvix_semaphore full;
	struct nanvix_semaphore empty;
	word_t data[BUFLEN*OBJSIZE/WORD_SIZE];
};

/**
 * @brief Task info.
 */
struct tdata
{
	int n;
	int tnum;
	struct buffer *buf;
	word_t data[OBJSIZE/WORD_SIZE];
} tdata[NTHREADS_MAX] ALIGN(CACHE_LINE_SIZE);

/**
 * @brief Buffers.
 */
static struct buffer buffers[NTHREADS_MAX/2];


/*============================================================================*
 * buffer_init()                                                              *
 *============================================================================*/

/**
 * @brief Initializes a buffer.
 */
static void buffer_init(struct buffer *buf, int nslots)
{
	buf->first = 0;
	buf->last = 0;
	nanvix_mutex_init(&buf->mutex);
	nanvix_semaphore_init(&buf->full, 0);
	nanvix_semaphore_init(&buf->empty, nslots - 1);
}

/*============================================================================*
 * buffer_put()                                                               *
 *============================================================================*/

/**
 * @brief Puts an object in a buffer.
 *
 * @param buf  Target buffer.
 * @param data Data to place in the buffer.
 */
static inline void buffer_put(struct buffer *buf, const word_t *data)
{
	memcopy(&buf->data[buf->first], data, OBJSIZE/WORD_SIZE);
	buf->first = (buf->first + OBJSIZE/WORD_SIZE)%(BUFLEN*OBJSIZE/WORD_SIZE);
}

/*============================================================================*
 * buffer_get()                                                               *
 *============================================================================*/

/**
 * @brief Gets an object from a buffer.
 *
 * @param buf  Target buffer.
 * @param data Location to store the data.
 */
static inline void buffer_get(struct buffer *buf, word_t *data)
{
	memcopy(data, &buf->data[buf->last], OBJSIZE/WORD_SIZE);
	buf->last = (buf->last + OBJSIZE/WORD_SIZE)%(BUFLEN*OBJSIZE/WORD_SIZE);
}

/*============================================================================*
 * producer()                                                                 *
 *============================================================================*/

/**
 * @brief Places objects in a shared buffer.
 */
static void *producer(void *arg)
{
	uint64_t t0, t1;
	struct tdata *t = arg;
	struct buffer *buf = t->buf;

	memfill(t->data, (word_t) -1, OBJSIZE/WORD_SIZE);

	perf_start(0, PERF_DCACHE_STALLS);
	perf_start(1, PERF_ICACHE_STALLS);
	t0 = stopwatch_read();

		do
		{
			nanvix_semaphore_down(&buf->empty);
				nanvix_mutex_lock(&buf->mutex);

					buffer_put(buf, t->data);

				nanvix_mutex_unlock(&buf->mutex);
			nanvix_semaphore_up(&buf->full);

		} while (--t->n > 0);

	t1 = stopwatch_read();
	perf_stop(1);
	perf_stop(0);

	if (iteration >= SKIP)
	{
		printf("%s %d %s %s %d %s %d %s %d %s %d %s %d %s %d %s %d",
			"[benchmarks][buffer]",
			iteration - SKIP,
			"p",
			"nthreads",
			NTHREADS,
			"tnum",
			t->tnum,
			"nobjects",
			NOBJECTS,
			"objsize",
			OBJSIZE,
			"cycles",
			UINT32(stopwatch_diff(t0, t1)),
			"d-stalls",
			UINT32(perf_read(0)),
			"i-stalls",
			UINT32(perf_read(1))
		);
	}

	return (NULL);
}

/*============================================================================*
 * consumer()                                                                 *
 *============================================================================*/

/**
 * @brief Removes objects from a buffer.
 */
static void *consumer(void *arg)
{
	uint64_t t0, t1;
	struct tdata *t = arg;
	struct buffer *buf = t->buf;

	memfill(t->data, 0, OBJSIZE/WORD_SIZE);

	perf_start(0, PERF_DCACHE_STALLS);
	perf_start(1, PERF_ICACHE_STALLS);
	t0 = stopwatch_read();

		do
		{
			nanvix_semaphore_down(&buf->full);
				nanvix_mutex_lock(&buf->mutex);

					buffer_get(buf, t->data);

				nanvix_mutex_unlock(&buf->mutex);
			nanvix_semaphore_up(&buf->empty);
		} while (--t->n > 0);

	t1 = stopwatch_read();
	perf_stop(1);
	perf_stop(0);

	if (iteration >= SKIP)
	{
		printf("%s %d %s %s %d %s %d %s %d %s %d %s %d %s %d %s %d",
			"[benchmarks][buffer]",
			iteration - SKIP,
			"c",
			"nthreads",
			NTHREADS,
			"tnum",
			t->tnum,
			"nobjects",
			NOBJECTS,
			"objsize",
			OBJSIZE,
			"cycles",
			UINT32(stopwatch_diff(t0, t1)),
			"d-stalls",
			UINT32(perf_read(0)),
			"i-stalls",
			UINT32(perf_read(1))
		);
	}

	return (NULL);
}

/*============================================================================*
 * benchmark_buffer()                                                         *
 *============================================================================*/

/**
 * @brief Buffer Benchmark Kernel
 *
 * @param nthreads Number of working threads.
 */
static void kernel_buffer(int nthreads)
{
	kthread_t tid[NTHREADS_MAX];

	/* Save kernel parameters. */
	NTHREADS = nthreads;

	/* Spawn threads. */
	for (iteration = 0; iteration < (NITERATIONS + SKIP); iteration++)
	{
		for (int i = 0; i < nthreads; i += 2)
		{
			struct buffer *buf;

			buffer_init(buf = &buffers[i/2], BUFLEN);

			tdata[i].tnum = i;
			tdata[i + 1].tnum = i + 1;
			tdata[i].n = tdata[i + 1].n = NOBJECTS;
			tdata[i].buf = tdata[i + 1].buf = buf;

			kthread_create(&tid[i], producer, &tdata[i]);
			kthread_create(&tid[i + 1], consumer, &tdata[i + 1]);
		}

		/* Wait for threads. */
		for (int i = 0; i < nthreads; i++)
			kthread_join(tid[i], NULL);
	}
}

/**
 * @brief Buffer Benchmark
 */
void benchmark_buffer(void)
{
#ifndef NDEBUG

	kernel_buffer(2);

#else

	for (int nthreads = NTHREADS_MIN; nthreads <= NTHREADS_MAX; nthreads += NTHREADS_STEP)
		kernel_buffer(nthreads);

#endif
}

#endif /* __benchmark_buffer__ */

