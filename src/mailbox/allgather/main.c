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

static struct final_results results;
static char message[MAILBOX_MSG_SIZE];

uint64_t build_footprint(int nodes[], int nnodes, int local)
{
	uint64_t footprint = 0ULL;

	for (int i = 0; i < nnodes; ++i)
		footprint |= (1ULL << nodes[i]);

	return (footprint & (~(1ULL << local)));
}

void do_work(int nodes[], int nnodes, int index)
{
	int local;
	int inbox;
	int outbox;
	int target;
	uint64_t expected, received;

	local = nodes[index];
	expected = build_footprint(nodes, nnodes, local);

	KASSERT((inbox = kmailbox_create(local)) >= 0);

		for (unsigned i = 0; i < NITERATIONS; ++i)
		{
			kmemset(message, (char) (local), MAILBOX_MSG_SIZE);

			/* Sends n-1 messages. */
			target = index;
			for (int j = 0; j < (nnodes - 1); ++j)
			{
				KASSERT((outbox = kmailbox_open(nodes[target])) >= 0);
					KASSERT(kmailbox_write(outbox, message, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
				KASSERT(kmailbox_close(outbox) == 0);

				target = (target + 1) % nnodes;
			}

			/* Receives n-1 messages. */
			received = 0ULL;
			for (int j = 0; j < (nnodes - 1); ++j)
			{
				kmemset(message, (char) (-1), MAILBOX_MSG_SIZE);
				KASSERT(kmailbox_read(inbox, message, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
				received |= (1ULL << message[0]);
			}

			KASSERT(expected == received);
		}

		KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_LATENCY, &results.latency) == 0);
		KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_VOLUME, &results.volume) == 0);

	KASSERT(kmailbox_unlink(inbox) == 0);
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
	int index;
	struct initial_arguments _args;
	int nodes[PROCESSOR_CLUSTERS_NUM];

	exchange_arguments(argc, argv, &_args);

	build_node_list(_args.nioclusters, _args.ncclusters, nodes);

	/* Filters the clusters involved. */
	if (cluster_is_io())
	{
		if (cluster_get_num() >= _args.nioclusters)
			return (0);
	}
	else
	{
		if (cluster_get_num() >= (PROCESSOR_IOCLUSTERS_NUM + _args.ncclusters))
			return (0);
	}

	/* Finds the node list index of the current cluster. */
	index = -1;
	for (int i = 0; i < (_args.nioclusters + _args.ncclusters); ++i)
	{
		if (nodes[i] == knode_get_num())
		{
			index = i;
			break;
		}
	}
	KASSERT(index != -1);

	barrier_setup(_args.nioclusters, _args.ncclusters);

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf(HLINE);

		/* Runs the kernel. */
		do_work(nodes, (_args.nioclusters + _args.ncclusters), index);

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf("[mailbox][allgather] Successfuly completed.");

		barrier();

		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		{
			uint64_t l0 = results.latency;
			uint64_t v0 = results.volume;

			receive_results(((_args.nioclusters - 1) + _args.ncclusters), &results);

			results.latency = results.latency < l0 ? l0 : results.latency;
			results.volume  = results.volume < v0  ? v0 : results.volume;

			print_results((_args.nioclusters + _args.ncclusters), NITERATIONS, &results);
		}
		else
			send_results(&results);

		barrier();

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf("[mailbox][allgather] Exit.");

	barrier_cleanup();

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf(HLINE);

	return (0);
}
