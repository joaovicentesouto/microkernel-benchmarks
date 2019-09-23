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

#define MESSAGE_MSG_MAX (1*MB)

void master(int message_size)
{
	int local;
	int remote;
	int portal_in;
	int portal_out;
	char message[MESSAGE_MSG_MAX];

	local  = knode_get_num();
	remote = (local + 1) % 2;

	kmemset(message, 0, message_size);

	portal_in = kportal_create(local);
	portal_out = kportal_open(local, remote);

		for (unsigned i = 0; i < NITERATIONS; i++)
		{
			kportal_allow(portal_in, remote);
			kportal_read(portal_in, message, message_size);

			kportal_write(portal_out, message, message_size);
		}

	kportal_close(portal_out);
	kportal_unlink(portal_in);
}

void slave(int message_size)
{
	int local;
	int remote;
	int portal_in;
	int portal_out;
	char message[MESSAGE_MSG_MAX];

	local  = knode_get_num();
	remote = (local + 1) % 2;

	portal_in = kportal_create(local);
	portal_out = kportal_open(local, remote);

	kmemset(message, 0, message_size);

		for (unsigned i = 0; i < NITERATIONS; i++)
		{
			kportal_write(portal_out, message, message_size);

			kportal_allow(portal_in, remote);
			kportal_read(portal_in, message, message_size);
		}

	kportal_close(portal_out);
	kportal_unlink(portal_in);
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
	int nclusters;
	int message_size;

	kprintf(HLINE);

	if (argc != 1)
		return (-EINVAL);

	nclusters    = __atoi(argv[0]);
	message_size = __atoi(argv[1]);

	if (cluster_get_num() < nclusters)
	{
		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
			master(message_size);
		else
			slave(message_size);
	}

	kprintf(HLINE);

	return (0);
}
