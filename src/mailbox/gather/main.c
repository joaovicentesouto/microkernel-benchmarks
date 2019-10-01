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

int build_footprint(int nodes[], int nslaves, int local)
{
	int footprint = 0;

	for (int i = 1; i < nslaves; ++i)
		if (nodes[i] != local)
			footprint |= (1ULL << nodes[i]);

	return (footprint);
}

void do_master(int nodes[], int nslaves)
{
	int local;
	int inbox;
	int expected, received;

	local = nodes[0];
	expected = build_footprint(nodes, (nslaves + 1), local);

	KASSERT((inbox = kmailbox_create(local)) >= 0);

		barrier();

		for (unsigned i = 0; i < NITERATIONS; ++i)
		{
			received = 0;

			/* Reads NSLAVES messages. */
			for (int j = 0; j < nslaves; ++j)
			{
				kmemset(message, ((char) -1), MAILBOX_MSG_SIZE);

				KASSERT(kmailbox_read(inbox, message, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);

				received |= (1ULL << message[0]);
			}

			KASSERT(expected == received);
		}

		barrier();

		KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_LATENCY, &results.latency) == 0);
		KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_VOLUME, &results.volume) == 0);

	KASSERT(kmailbox_unlink(inbox) == 0);
}

void do_slave(int nodes[])
{
	int local;
	int remote;
	int outbox;

	local  = knode_get_num();
	remote = nodes[0];

	kmemset(message, (char) local, MAILBOX_MSG_SIZE);

	KASSERT((outbox = kmailbox_open(remote)) >= 0);

		barrier();

		for (unsigned i = 0; i < NITERATIONS; i++)
			KASSERT(kmailbox_write(outbox, message, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
		
		barrier();

	KASSERT(kmailbox_close(outbox) == 0);
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

		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
			do_master(nodes, ((_args.nioclusters - 1) + _args.ncclusters));
		else
			do_slave(nodes);

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf("[mailbox][gather] Successfuly completed.");

		barrier();

		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
			print_results(((_args.nioclusters - 1) + _args.ncclusters), NITERATIONS, &results);

		barrier();

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf("[mailbox][gather] Exit.");

	barrier_cleanup();

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf(HLINE);

	return (0);
}
