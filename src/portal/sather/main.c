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
#include <nanvix/sys/sync.h>
#include <nanvix/sys/noc.h>
#include <stdint.h>
#include <kbench.h>

#define MESSAGE_MSG_MAX (4096)

void do_master(int nodes[], int nslaves, int message_size)
{
	int local;
	int syncid;
	int sync_nodelist[5];
	int portalids[4];
	char message[PROCESSOR_CLUSTERS_NUM * MESSAGE_MSG_MAX];

	local = nodes[0];
	sync_nodelist[0] = nodes[0];

	for (int i = 0; i < nslaves; ++i)
		kmemset(&message[i * message_size], (char) nodes[i + 1], message_size);

	barrier();

	for (unsigned i = 0; i < NITERATIONS; ++i)
	{
		for (int j = 0; j < nslaves; j += 4)
		{
			int index = 0;

			/* Opens connectors. */
			for (int remote = (j + 1); remote <= (j + 4) && remote <= nslaves; ++remote)
			{
				KASSERT((portalids[index++] = kportal_open(local, nodes[remote])) >= 0);
				sync_nodelist[index] = nodes[remote];
			}
			KASSERT((syncid = ksync_open(sync_nodelist, ++index, SYNC_ONE_TO_ALL)) >= 0);

				/* Releases slaves to read (send permission ack to sender). */
				KASSERT(ksync_signal(syncid) == 0);

				/* Sends the message for current slaves. */
				for (int k = 0; k < (index - 1); ++k)
				{
					KASSERT(
						kportal_write(
							portalids[k],
							&message[(j + k) * message_size],
							message_size
						) == message_size
					);
				}

			/* Closes connectors. */
			KASSERT(ksync_close(syncid) == 0);
			for (int k = 0; k < (index - 1); ++k)
				KASSERT(kportal_close(portalids[k]) == 0);
		}
	}
}

void do_slave(int nodes[], int message_size)
{
	int local;
	int remote;
	int syncid;
	int sync_nodelist[2];
	int portalid;
	char message[PROCESSOR_CLUSTERS_NUM * MESSAGE_MSG_MAX];

	local  = knode_get_num();
	remote = nodes[0];
	sync_nodelist[0] = nodes[0];
	sync_nodelist[1] = local;

	kmemset(message, 0, message_size);

	KASSERT((syncid = ksync_create(sync_nodelist, 2, SYNC_ONE_TO_ALL)) >= 0);
	KASSERT((portalid = kportal_create(local)) >= 0);

		barrier();

		for (unsigned i = 0; i < NITERATIONS; ++i)
		{
			kmemset(message, 0, message_size);

			KASSERT(ksync_wait(syncid) == 0);

			KASSERT(kportal_allow(portalid, remote) == 0);
			KASSERT(kportal_read(portalid, message, message_size) == message_size);

			for (int j = 0; j < message_size; ++j)
			{
				char value = (char) local;

				KASSERT(message[j] == value);
			}
		}

	KASSERT(kportal_unlink(portalid) == 0);
	KASSERT(ksync_unlink(syncid) == 0);
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
	int message_size;
	int nodes[PROCESSOR_CLUSTERS_NUM];

	if (argc != 3)
	{
		kprintf("[portal][sather] Invalid arguments! (%d)", argc);
		return (-EINVAL);
	}

	ncclusters   = atoi(argv[1]);
	message_size = atoi(argv[2]);

	if (ncclusters == 0 || ncclusters > 16 || message_size == 0)
	{
		kprintf("[portal][sather] Bad arguments! (%d, %d)", ncclusters, message_size);
		return (-EINVAL);
	}

	/* Builds node list. */
	nodes[0] = 4;
	for (int i = 0; i < ncclusters; ++i)
		nodes[(i + 1)] = 8 + i;

	/* Filters the clusters involved. */
	if (cluster_is_compute())
	{
		if (knode_get_num() >= 8 + ncclusters)
			return (0);
	}

	kprintf(HLINE);

	barrier_setup(2, ncclusters);

		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
			barrier();
		else
		{
			if (knode_get_num() == nodes[0])
				do_master(nodes, ncclusters, message_size);
			else
				do_slave(nodes, message_size);
			
			kprintf("[portal][sather] Successfuly completed.");
		}

		barrier();

		kprintf("[portal][sather] Exit.");

	barrier_cleanup();

	kprintf(HLINE);

	return (0);
}
