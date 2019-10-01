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

#ifdef __node__
	static char message[MESSAGE_MSG_MAX];
#else
	static char message[PROCESSOR_CLUSTERS_NUM * MESSAGE_MSG_MAX];
#endif

void do_master(int nodes[], int nslaves, int message_size)
{
	int local;
	int portal_in;

	local = nodes[0];

	kmemset(message, 0, (nslaves * message_size));

	KASSERT((portal_in = kportal_create(local)) >= 0);

		barrier();

		for (unsigned i = 0; i < NITERATIONS; ++i)
		{
			kmemset(message, 0, message_size);

			/* Reads NSLAVES messages. */
			for (int j = 1; j <= nslaves; ++j)
			{
				KASSERT(kportal_allow(portal_in, nodes[j]) == 0);
				KASSERT(
					kportal_read(
						portal_in,
						&message[((j - 1) * message_size)],
						message_size
					) == message_size
				);
			}

			/* Checks transfer. */
			for (int j = 1; j <= nslaves; ++j)
			{
				char value = nodes[j];
				char * curr_message = &message[((j - 1) * message_size)];

				for (int k = 0; k < message_size; ++k)
					KASSERT(curr_message[k] == value);
			}
		}

		barrier();

		KASSERT(kportal_ioctl(portal_in, PORTAL_IOCTL_GET_LATENCY, &results.latency) == 0);
		KASSERT(kportal_ioctl(portal_in, PORTAL_IOCTL_GET_VOLUME, &results.volume) == 0);

	KASSERT(kportal_unlink(portal_in) == 0);
}

void do_slave(int nodes[], int message_size)
{
	int local;
	int remote;
	int portal_out;

	local  = knode_get_num();
	remote = nodes[0];

	kmemset(message, (char) local, message_size);

	KASSERT((portal_out = kportal_open(local, remote)) >= 0);

		barrier();

		for (unsigned i = 0; i < NITERATIONS; i++)
			KASSERT(kportal_write(portal_out, message, message_size) == message_size);
		
		barrier();

	KASSERT(kportal_close(portal_out) == 0);
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
			do_master(nodes, ((_args.nioclusters - 1) + _args.ncclusters), _args.message_size);
		else
			do_slave(nodes, _args.message_size);

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf("[portal][gather] Successfuly completed.");

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
		kprintf("[portal][gather] Exit.");

	barrier_cleanup();

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf(HLINE);

	return (0);
}
