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

void do_master(int nodes[], int nslaves)
{
	int local;
	int outboxes[4];

	local = nodes[0];

	kmemset(message, local, MAILBOX_MSG_SIZE);

	barrier();

	for (unsigned i = 0; i < NITERATIONS; ++i)
	{
		for (int j = 0; j < nslaves; j += 4)
		{
			int index = 0;

			/* Opens connectors. */
			for (int remote = (j + 1); remote <= (j + 4) && remote <= nslaves; ++remote)
				KASSERT((outboxes[index++] = kmailbox_open(nodes[remote])) >= 0);

			/* Sends the message for current slaves. */
			for (int k = 0; k < index; ++k)
				KASSERT(kmailbox_write(outboxes[k], message, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);

			/* Closes connectors. */
			for (int k = 0; k < index; ++k)
				KASSERT(kmailbox_close(outboxes[k]) == 0);
		}
	}
}

void do_slave(int nodes[])
{
	int inbox;
	int local;
	int remote;

	local  = knode_get_num();
	remote = nodes[0];

	KASSERT((inbox = kmailbox_create(local)) >= 0);

		barrier();

		for (unsigned i = 0; i < NITERATIONS; ++i)
		{
			kmemset(message, 0, MAILBOX_MSG_SIZE);

			KASSERT(kmailbox_read(inbox, message, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);

			for (int j = 0; j < MAILBOX_MSG_SIZE; ++j)
				KASSERT(message[j] == remote);
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
	struct initial_arguments _args;
	int nodes[PROCESSOR_CLUSTERS_NUM];

	exchange_arguments(argc, argv, &_args);

	/* Builds node list. */
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

	barrier_setup(_args.nioclusters, _args.ncclusters);

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf(HLINE);

		if (knode_get_num() == nodes[0])
			do_master(nodes, (_args.nioclusters - 1) + _args.ncclusters);
		else
			do_slave(nodes);

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf("[mailbox][broadcast] Successfuly completed.");

	barrier();

		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		{
			receive_results(((_args.nioclusters - 1) + _args.ncclusters), &results);

			print_results(((_args.nioclusters - 1) + _args.ncclusters), NITERATIONS, &results);
		}
		else
			send_results(&results);

		barrier();

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf("[mailbox][broadcast] Exit.");

	barrier_cleanup();

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf(HLINE);

	return (0);
}
