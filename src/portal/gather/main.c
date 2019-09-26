/*
 * MIT License
 *
 * Copyright(c) 2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
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

#include <nanvix/sys/portal.h>
#include <nanvix/sys/noc.h>
#include <stdint.h>
#include <kbench.h>

#define MESSAGE_MSG_MAX (4096)

static int nodeids[PROCESSOR_CLUSTERS_NUM];

void master(int nslaves, int message_size)
{
	int local;
	int portalid;
	char message[PROCESSOR_CLUSTERS_NUM * MESSAGE_MSG_MAX];

	local = nodeids[0];

	kmemset(message, 0, (nslaves * message_size));

	KASSERT((portalid = kportal_create(local)) >= 0);

		fence();

		for (unsigned i = 0; i < NITERATIONS; ++i)
		{
			kmemset(message, 0, message_size);

			/* Reads NSLAVES messages. */
			for (int j = 1; j <= nslaves; ++j)
			{
				KASSERT(kportal_allow(portalid, nodeids[j]) == 0);
				KASSERT(
					kportal_read(
						portalid,
						&message[((j - 1) * message_size)],
						message_size
					) == message_size
				);
			}

			/* Checks transfer. */
			for (int j = 1; j <= nslaves; ++j)
			{
				char value = nodeids[j];
				char * curr_message = &message[((j - 1) * message_size)];

				kprintf("I WILL SEE nodeid:%d => %d", (int) value, (int) curr_message[0]);

				for (int k = 0; k < message_size; ++k)
					KASSERT(curr_message[k] == value);
			}
		}

		fence();

	KASSERT(kportal_unlink(portalid) == 0);
}

void slave(int message_size)
{
	int local;
	int remote;
	int portalid;
	char message[MESSAGE_MSG_MAX];

	local  = knode_get_num();
	remote = nodeids[0];

	kmemset(message, (char) local, message_size);

	kprintf("+ I WILL SEND %d", (int) message[0]);

	KASSERT((portalid = kportal_open(local, remote)) >= 0);

		fence();

		for (unsigned i = 0; i < NITERATIONS; i++)
			KASSERT(kportal_write(portalid, message, message_size) == message_size);
		
		fence();

	KASSERT(kportal_close(portalid) == 0);
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance Monitoring Overhead Benchmark
 *
 * @param argc Argument counter.
 * @param argv Argument variables.
 */
int main(int argc, const char *argv[])
{
	int ncclusters;
	int nioclusters;
	int message_size;

	if (argc != 3)
	{
		kprintf("[portal][gather] Invalid arguments! (%d)", argc);
		return (-EINVAL);
	}

	nioclusters  = __atoi(argv[0]);
	ncclusters   = __atoi(argv[1]);
	message_size = __atoi(argv[2]);

	if (ncclusters > 16 || message_size == 0)
	{
		kprintf("[portal][gather] Bad arguments! (%d, %d)", ncclusters, message_size);
		return (-EINVAL);
	}

	build_node_list(nioclusters, ncclusters, nodeids);

	/* Filters the clusters involved. */
	if (cluster_is_compute())
	{
		if (knode_get_num() >= 8 + ncclusters)
			return (0);
	}

	/* Filters the clusters involved. */
	if (cluster_is_io())
	{
		if (cluster_get_num() >= nioclusters)
			return (0);
	}
	else
	{
		if (cluster_get_num() >= (PROCESSOR_IOCLUSTERS_NUM + ncclusters))
			return (0);
	}

	kprintf(HLINE);

	fence_setup(1, ncclusters);

		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
			master(((nioclusters - 1) + ncclusters), message_size);
		else
			slave(message_size);

		kprintf("[portal][gather] Successfuly completed.");

	fence_cleanup();

	kprintf(HLINE);

	return (0);
}
