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
#include <nanvix/sys/noc.h>
#include <stdint.h>
#include <kbench.h>

#define MESSAGE_MSG_MAX (4096)

static int nodeids[2];

void master(int message_size)
{
	int local;
	int remote;
	int portal_in;
	int portal_out;
	char message[MESSAGE_MSG_MAX];

	local  = nodeids[0];
	remote = nodeids[1];

	kmemset(message, 0, message_size);

	KASSERT((portal_in = kportal_create(local)) >= 0);
	KASSERT((portal_out = kportal_open(local, remote)) >= 0);

		fence();

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

void slave(int message_size)
{
	int local;
	int remote;
	int portal_in;
	int portal_out;
	char message[MESSAGE_MSG_MAX];

	local  = nodeids[1];
	remote = nodeids[0];

	kmemset(message, 0, message_size);

	KASSERT((portal_in = kportal_create(local)) >= 0);
	KASSERT((portal_out = kportal_open(local, remote)) >= 0);

		fence();

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
	int message_size;

	if (argc != 3)
	{
		kprintf("[portal][pingpong] Invalid arguments! (%d)", argc);
		return (-EINVAL);
	}

	message_size = __atoi(argv[2]);

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
			master(message_size);
		else
			slave(message_size);

		kprintf("[portal][pingpong] Successfuly completed.");

	fence_cleanup();

	kprintf(HLINE);

	return (0);
}
