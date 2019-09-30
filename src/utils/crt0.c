/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
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

#include <nanvix/sys/dev.h>
#include <nanvix/sys/thread.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/portal.h>
#include <nanvix/sys/sync.h>
#include <nanvix/sys/signal.h>
#include <stddef.h>
#include <kbench.h>

struct __mailbox_args
{
	int nioclusters;
	int ncclusters;
	int message_size;
	char unused[(MAILBOX_MSG_SIZE - (3 * sizeof(int)))];
};

/*
 * Main routine.
 */
extern int main(int argc, const char *argv[]);

/**
 *  Terminates the calling process.
 */
NORETURN  void ___nanvix_exit(int status)
{
	((void) status);

	kshutdown();
	UNREACHABLE();
}

/*
 * Entry point of the program.
 */
void ___start(int argc, const char *argv[], char **envp)
{
	int ret;

	UNUSED(envp);

	barrier_setup(PROCESSOR_IOCLUSTERS_NUM, PROCESSOR_CCLUSTERS_NUM);

		if (cluster_get_num() != PROCESSOR_CLUSTERNUM_MASTER)
		{
			kprintf("    1. dummy");

				timer(5 * CLUSTER_FREQ);
				timer(CLUSTER_FREQ);

			kprintf("    2. dummy");
		}

		barrier();

	barrier_cleanup();

	ret = main(argc, argv);

	/* Power off. */
	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		___nanvix_exit(ret);

	/* Loop forever. */
	UNREACHABLE();
}
