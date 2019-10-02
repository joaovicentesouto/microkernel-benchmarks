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

#include <stdint.h>
#include <kbench.h>

#define MESSAGE_MSG_MAX (64 * KB)

static struct final_results results;
static char message[MESSAGE_MSG_MAX];

void do_master(int nodes[], int nslaves, int message_size)
{
	int local;
	int syncid;
	int sync_nodelist[5];
	int portals_out[4];

	local = nodes[0];
	sync_nodelist[0] = nodes[0];

	kmemset(message, 1, message_size);

		barrier();

		for (unsigned i = 0; i < NITERATIONS; ++i)
		{
			kprintf("Iteration %d/%d", i, NITERATIONS);

			for (int j = 0; j < nslaves; j += 4)
			{
				int index = 0;

				/* Opens connectors. */
				for (int remote = (j + 1); remote <= (j + 4) && remote <= nslaves; ++remote)
				{
					KASSERT((portals_out[index++] = kportal_open(local, nodes[remote])) >= 0);
					sync_nodelist[index] = nodes[remote];
				}

				KASSERT((syncid = ksync_open(sync_nodelist, ++index, SYNC_ONE_TO_ALL)) >= 0);

				/* Releases slaves to read (send permission ack to sender). */
				KASSERT(ksync_signal(syncid) == 0);

				/* Sends the message for current slaves. */
				for (int k = 0; k < (index - 1); ++k)
					KASSERT(kportal_write(portals_out[k], message, message_size) == message_size);

				/* Closes connectors. */
				KASSERT(ksync_close(syncid) == 0);
				for (int k = 0; k < (index - 1); ++k)
					KASSERT(kportal_close(portals_out[k]) == 0);
			}
		}
}

void do_slave(int nodes[], int message_size)
{
	int local;
	int remote;
	int syncid;
	int sync_nodelist[2];
	int portal_in;

	local  = knode_get_num();
	remote = nodes[0];
	sync_nodelist[0] = nodes[0];
	sync_nodelist[1] = local;

	kmemset(message, 0, message_size);

	KASSERT((syncid = ksync_create(sync_nodelist, 2, SYNC_ONE_TO_ALL)) >= 0);
	KASSERT((portal_in = kportal_create(local)) >= 0);

		barrier();

		for (unsigned i = 0; i < NITERATIONS; ++i)
		{
			kmemset(message, 0, message_size);

			KASSERT(ksync_wait(syncid) == 0);

			KASSERT(kportal_allow(portal_in, remote) == 0);
			KASSERT(kportal_read(portal_in, message, message_size) == message_size);

			for (int j = 0; j < message_size; ++j)
				KASSERT(message[j] == 1);
		}

		KASSERT(kportal_ioctl(portal_in, PORTAL_IOCTL_GET_LATENCY, &results.latency) == 0);
		KASSERT(kportal_ioctl(portal_in, PORTAL_IOCTL_GET_VOLUME, &results.volume) == 0);

	KASSERT(kportal_unlink(portal_in) == 0);
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
	struct initial_arguments _args;
	int nodes[PROCESSOR_CLUSTERS_NUM];

	exchange_arguments(argc, argv, &_args);

	/* Builds node list. */
	nodes[0] = 4;
	for (int i = 0; i < _args.ncclusters; ++i)
		nodes[(i + 1)] = 8 + i;

	/* Filters the clusters involved. */
	if (cluster_is_compute())
	{
		if (knode_get_num() >= 8 + _args.ncclusters)
			return (0);
	}

	barrier_setup(2, _args.ncclusters);

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf(HLINE);

		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
			barrier();
		else 
		{
			if (knode_get_num() == nodes[0])
				do_master(nodes, _args.ncclusters, _args.message_size);
			else
				do_slave(nodes, _args.message_size);
		}

		barrier();

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf("[portal][broadcast] Successfuly completed.");

		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		{
			receive_results(_args.ncclusters, &results);
			print_results("portal", "broadcast", _args.ncclusters, NITERATIONS, &results);
		}
		else
			send_results(&results);

		barrier();

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf("[portal][broadcast] Exit.");

	barrier_cleanup();

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf(HLINE);

	return (0);
}
