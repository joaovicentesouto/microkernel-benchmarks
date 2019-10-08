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

#include <stddef.h>
#include <kbench.h>

void send_results(struct final_results * results)
{
	int outbox;

	/* Needs to be a slave. */
	KASSERT((cluster_get_num() != PROCESSOR_CLUSTERNUM_MASTER));

	barrier();

	KASSERT((outbox = kmailbox_open(PROCESSOR_NODENUM_MASTER)) >= 0);
		KASSERT(kmailbox_write(outbox, results, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
	KASSERT(kmailbox_close(outbox) == 0);
}

void receive_results(int nslaves, struct final_results * results)
{
	int inbox;
	uint64_t worst_volume;
	uint64_t worst_latency;

	/* Needs to be the master. */
	KASSERT((cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER));

	worst_volume  = 0ULL;
	worst_latency = 0ULL;

	__stdmailbox_setup();

		barrier();

		KASSERT((inbox = stdinbox_get()) >= 0);

		timer(CLUSTER_FREQ);

		for (int i = 0; i < nslaves; ++i)
		{
			KASSERT(kmailbox_read(inbox, results, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);

			if (worst_volume < results->volume)
				worst_volume = results->volume;

			if (worst_latency < results->latency)
				worst_latency = results->latency;			
		}

	__stdmailbox_cleanup();

	results->volume  = worst_volume;
	results->latency = worst_latency;
}

void print_results(
	char * abstraction,
	char * kernel,
	int nclusters,
	struct final_results * results
)
{
	kprintf("header;%s;clusters;latency;volume", kernel);
	kprintf(
		"%s;%s;%d;%ld;%ld",
		abstraction,
		kernel,
		nclusters,
		results->latency,
		results->volume
	);
}