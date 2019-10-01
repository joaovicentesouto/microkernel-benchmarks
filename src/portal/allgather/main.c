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

#define MESSAGE_MSG_MAX (64 * KB)

static struct final_results results;
static char message[PROCESSOR_CLUSTERS_NUM * MESSAGE_MSG_MAX];

void do_work(int nodes[], int nnodes, int index, int message_size)
{
	int local;
	int read_from;
	int write_to;
	int portal_in;
	int portal_out;

	local = nodes[index];

	KASSERT((portal_in = kportal_create(local)) >= 0);

		for (unsigned i = 0; i < NITERATIONS; ++i)
		{
			/* Cleans the buffer. */
			kmemset(message, (-1), nnodes * message_size);

			/* Prepares the transfer data. */
			kmemset(&message[index * message_size], (char) local, message_size);

			for (int j = 1; j < nnodes; ++j)
			{
				read_from = (index - j) < 0 ? (nnodes + (index - j)) : (index - j);
				write_to  = (index + j) % nnodes;

				KASSERT((portal_out = kportal_open(local, nodes[write_to])) >= 0);

					barrier();

					KASSERT(kportal_allow(portal_in, nodes[read_from]) == 0);
					KASSERT(
						kportal_aread(
							portal_in,
							&message[read_from * message_size],
							message_size
						) == message_size
					);

						KASSERT(
							kportal_write(
								portal_out,
								&message[index * message_size],
								message_size
							) == message_size
						);

					KASSERT(kportal_wait(portal_in) == 0);

				KASSERT(kportal_close(portal_out) == 0);
			}

			for (int j = 0; j < nnodes; ++j)
			{
				char value = (char) nodes[j];
				char * curr_message = &message[(j * message_size)];

				for (int k = 0; k < message_size; ++k)
					KASSERT(curr_message[k] == value);
			}
		}

		KASSERT(kportal_ioctl(portal_in, PORTAL_IOCTL_GET_LATENCY, &results.latency) == 0);
		KASSERT(kportal_ioctl(portal_in, PORTAL_IOCTL_GET_VOLUME, &results.volume) == 0);

	KASSERT(kportal_unlink(portal_in) == 0);
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
		do_work(nodes, (_args.nioclusters + _args.ncclusters), index, _args.message_size);

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf("[portal][allgather] Successfuly completed.");

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
		kprintf("[portal][allgather] Exit.");

	barrier_cleanup();

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf(HLINE);

	return (0);
}
