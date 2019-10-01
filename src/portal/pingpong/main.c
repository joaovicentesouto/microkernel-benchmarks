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

void do_master(int nodes[], int message_size)
{
	int local;
	int remote;
	int portal_in;
	int portal_out;

	local  = nodes[0];
	remote = nodes[1];

	kmemset(message, 0, message_size);

	KASSERT((portal_in = kportal_create(local)) >= 0);
	KASSERT((portal_out = kportal_open(local, remote)) >= 0);

		barrier();

		for (unsigned i = 0; i < NITERATIONS; i++)
		{
			kmemset(message, 0, message_size);

			KASSERT(kportal_allow(portal_in, remote) == 0);
			KASSERT(kportal_read(portal_in, message, message_size) == message_size);

			for (int j = 0; j < message_size; j++)
			{
				KASSERT(message[j] == 2);
				message[j] = 1;
			}

			KASSERT(kportal_write(portal_out, message, message_size) >= message_size);
		}

	KASSERT(kportal_close(portal_out) == 0);
	KASSERT(kportal_unlink(portal_in) == 0);
}

void do_slave(int nodes[], int message_size)
{
	int local;
	int remote;
	int portal_in;
	int portal_out;

	local  = nodes[1];
	remote = nodes[0];

	kmemset(message, 0, message_size);

	KASSERT((portal_in = kportal_create(local)) >= 0);
	KASSERT((portal_out = kportal_open(local, remote)) >= 0);

		barrier();

		for (unsigned i = 0; i < NITERATIONS; i++)
		{
			for (int j = 0; j < message_size; j++)
				message[j] = 2;

			KASSERT(kportal_write(portal_out, message, message_size) >= message_size);

			kmemset(message, 0, message_size);

			KASSERT(kportal_allow(portal_in, remote) == 0);
			KASSERT(kportal_read(portal_in, message, message_size) == message_size);

			for (int j = 0; j < message_size; j++)
				KASSERT(message[j] == 1);
		}

		KASSERT(kportal_ioctl(portal_in, PORTAL_IOCTL_GET_LATENCY, &results.latency) == 0);
		KASSERT(kportal_ioctl(portal_in, PORTAL_IOCTL_GET_VOLUME, &results.volume) == 0);

	KASSERT(kportal_close(portal_out) == 0);
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
	int nodes[2];
	struct initial_arguments _args;

	exchange_arguments(argc, argv, &_args);

	build_node_list(1, 1, nodes);

	/* Filters the clusters involved. */
	if (cluster_is_io())
	{
		if (knode_get_num() != nodes[0])
			return (0);
	}
	else
	{
		if (knode_get_num() != nodes[1])
			return (0);
	}

	barrier_setup(1, 1);

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf(HLINE);

		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
			do_master(nodes, _args.message_size);
		else
			do_slave(nodes, _args.message_size);

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf("[portal][pingpong] Successfuly completed.");

		barrier();

		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		{
			uint64_t l0 = results.latency;
			uint64_t v0 = results.volume;
			
			receive_results(1, &results);

			results.latency = results.latency < l0 ? l0 : results.latency;
			results.volume  = results.volume < v0  ? v0 : results.volume;

			print_results(2, NITERATIONS, &results);
		}
		else
			send_results(&results);

		barrier();

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf("[portal][pingpong] Exit.");

	barrier_cleanup();

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf(HLINE);

	return (0);
}
