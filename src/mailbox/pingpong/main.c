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

static int nodeids[2];
struct final_results results;

void do_master_results(void)
{
	uint64_t l0 = results.latency;
	uint64_t v0 = results.volume;
	
	receive_results(1, &results);

	results.latency = results.latency < l0 ? l0 : results.latency;
	results.volume  = results.volume < v0  ? v0 : results.volume;

	print_results("mailbox", "pingpong", 2, &results);
}

void do_master(void)
{
	int local;
	int remote;
	int inbox;
	int outbox;
	char message_in[MAILBOX_MSG_SIZE];
	char message_out[MAILBOX_MSG_SIZE];

	local  = nodeids[0];
	remote = nodeids[1];

	kmemset(message_out, 1, MAILBOX_MSG_SIZE);

	for (unsigned i = 1; i <= NITERATIONS; ++i)
	{
		kprintf("Iteration %d/%d", i, NITERATIONS);

		KASSERT((inbox = kmailbox_create(local)) >= 0);
		KASSERT((outbox = kmailbox_open(remote)) >= 0);

			kmemset(message_in, 0, MAILBOX_MSG_SIZE);

			barrier();

			KASSERT(kmailbox_write(outbox, message_out, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
			KASSERT(kmailbox_read(inbox,   message_in,  MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);

			for (unsigned j = 0; j < MAILBOX_MSG_SIZE; ++j)
				KASSERT(message_in[j] == 2);
			
			KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_LATENCY, &results.latency) == 0);
			KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_VOLUME, &results.volume) == 0);

		KASSERT(kmailbox_close(outbox) == 0);
		KASSERT(kmailbox_unlink(inbox) == 0);

		do_master_results();
	}
}

void do_slave_results(void)
{
	send_results(&results);
}

void do_slave(void)
{
	int local;
	int remote;
	int inbox;
	int outbox;
	char message_in[MAILBOX_MSG_SIZE];
	char message_out[MAILBOX_MSG_SIZE];

	local  = nodeids[1];
	remote = nodeids[0];

	kmemset(message_out, 2, MAILBOX_MSG_SIZE);

	for (unsigned i = 1; i <= NITERATIONS; ++i)
	{
		KASSERT((inbox = kmailbox_create(local)) >= 0);
		KASSERT((outbox = kmailbox_open(remote)) >= 0);

			kmemset(message_in, 0, MAILBOX_MSG_SIZE);

			barrier();

			KASSERT(kmailbox_read(inbox,   message_in,  MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
			KASSERT(kmailbox_write(outbox, message_out, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);

			for (unsigned j = 0; j < MAILBOX_MSG_SIZE; ++j)
				KASSERT(message_in[j] == 1);

			KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_LATENCY, &results.latency) == 0);
			KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_VOLUME, &results.volume) == 0);

		KASSERT(kmailbox_close(outbox) == 0);
		KASSERT(kmailbox_unlink(inbox) == 0);

		do_slave_results();
	}
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

	exchange_arguments(argc, argv, &_args);

	build_node_list(1, 1, nodeids);

	/* Filters the clusters involved. */
	if (cluster_is_io())
	{
		if (knode_get_num() != nodeids[0])
			return (0);
	}
	else
	{
		if (knode_get_num() != nodeids[1])
			return (0);
	}

	barrier_setup(1, 1);

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf(HLINE);

		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
			do_master();
		else
			do_slave();

	barrier();

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf("[mailbox][pingpong] successfuly completed.");

	barrier_cleanup();

	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		kprintf(HLINE);

	return (0);
}
