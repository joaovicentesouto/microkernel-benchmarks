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

#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/noc.h>
#include <stdint.h>
#include <kbench.h>

static int nodeids[2];

void master(void)
{
	int local;
	int remote;
	int mbx_in;
	int mbx_out;
	char message[MAILBOX_MSG_SIZE];

	local  = nodeids[0];
	remote = nodeids[1];

	KASSERT((mbx_in = kmailbox_create(local)) >= 0);
	KASSERT((mbx_out = kmailbox_open(remote)) >= 0);

		fence();

		for (unsigned i = 0; i < NITERATIONS; i++)
		{
			kmemset(message, 1, MAILBOX_MSG_SIZE);

			KASSERT(kmailbox_awrite(mbx_out, message, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
			KASSERT(kmailbox_wait(mbx_out) == 0);

			kmemset(message, 0, MAILBOX_MSG_SIZE);

			KASSERT(kmailbox_aread(mbx_in, message, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
			KASSERT(kmailbox_wait(mbx_in) == 0);

			for (unsigned j = 0; j < MAILBOX_MSG_SIZE; ++j)
				KASSERT(message[j] == 2);
		}

	KASSERT(kmailbox_close(mbx_out) == 0);
	KASSERT(kmailbox_unlink(mbx_in) == 0);
}

void slave(void)
{
	int local;
	int remote;
	int mbx_in;
	int mbx_out;
	char message[MAILBOX_MSG_SIZE];

	local  = nodeids[1];
	remote = nodeids[0];

	KASSERT((mbx_in = kmailbox_create(local)) >= 0);
	KASSERT((mbx_out = kmailbox_open(remote)) >= 0);

		fence();

		for (unsigned i = 0; i < NITERATIONS; i++)
		{
			kmemset(message, 0, MAILBOX_MSG_SIZE);

			KASSERT(kmailbox_aread(mbx_in, message, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
			KASSERT(kmailbox_wait(mbx_in) == 0);

			for (unsigned j = 0; j < MAILBOX_MSG_SIZE; ++j)
				KASSERT(message[j] == 1);

			kmemset(message, 2, MAILBOX_MSG_SIZE);

			KASSERT(kmailbox_awrite(mbx_out, message, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
			KASSERT(kmailbox_wait(mbx_out) == 0);
		}

	KASSERT(kmailbox_close(mbx_out) == 0);
	KASSERT(kmailbox_unlink(mbx_in) == 0);
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
	UNUSED(argc);
	UNUSED(argv);

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

	kprintf(HLINE);

	fence_setup(1, 1);

		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
			master();
		else
			slave();

		kprintf("[mailbox] Ping Pong successfuly completed.");

	fence_cleanup();

	kprintf(HLINE);

	return (0);
}
