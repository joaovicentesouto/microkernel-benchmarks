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
	int nclusters;
	int message_size;
	char unused[(MAILBOX_MSG_SIZE - (2 * sizeof(int)))];
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
 * Build a list of the node ids.
 */
void build_node_list(int nioclusters, int ncclusters, int *nodes)
{
	int base;
	int step;
	int index;
	int max_clusters;

	index = 0;

	/* Build IDs of the IO Clusters. */
	base         = 0;
	max_clusters = PROCESSOR_IOCLUSTERS_NUM;
	step         = (PROCESSOR_NOC_IONODES_NUM / PROCESSOR_IOCLUSTERS_NUM);

	for (int i = 0; i < max_clusters && i < nioclusters; i++, index++)
		nodes[index] = base + (i * step);

	/* Build IDs of the Compute Clusters. */
	base         = PROCESSOR_IOCLUSTERS_NUM * (PROCESSOR_NOC_IONODES_NUM / PROCESSOR_IOCLUSTERS_NUM);
	max_clusters = PROCESSOR_CCLUSTERS_NUM;
	step         = (PROCESSOR_NOC_CNODES_NUM / PROCESSOR_CCLUSTERS_NUM);

	for (int i = 0; i < max_clusters && i < ncclusters; i++, index++)
		nodes[index] = base + (i * step);
}

/*
 * Entry point of the program.
 */
void ___start(int argc, const char *argv[], char **envp)
{
	int ret;
	// uint64_t t0, cicles, limit;
	int _argc;
	char *_argv[1];
	char default_argv[2][100];
	struct __mailbox_args __mailbox_args;

	UNUSED(argc);
	UNUSED(envp);

	__mailbox_args.nclusters = 0;

	kprintf(" crt0 SETUP");
	fence_setup(PROCESSOR_IOCLUSTERS_NUM, PROCESSOR_CCLUSTERS_NUM);
	__stdmailbox_setup();

		if (cluster_get_num() != PROCESSOR_CLUSTERNUM_MASTER)
		{
			// int dummy = 20;
			// while (dummy--)
				kprintf("    dummy");

			kprintf("    1. dummy");

				timer(5 * CLUSTER_FREQ);

			kprintf("    2. dummy");
		}

		fence();

		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		{
			int outbox;
			int nodenum;

			nodenum = 0;
			__mailbox_args.nclusters    = __atoi(argv[1]);
			__mailbox_args.message_size = __atoi(argv[2]);

			for (int i = 0; i < PROCESSOR_IOCLUSTERS_NUM; i++)
			{
				outbox = kmailbox_open(nodenum);
				kmailbox_write(outbox, &__mailbox_args, MAILBOX_MSG_SIZE);
				kmailbox_close(outbox);

				nodenum += (PROCESSOR_NOC_IONODES_NUM / PROCESSOR_IOCLUSTERS_NUM);
			}

			for (int i = PROCESSOR_IOCLUSTERS_NUM; i < (PROCESSOR_IOCLUSTERS_NUM + PROCESSOR_CCLUSTERS_NUM); i++)
			{
				outbox = kmailbox_open(nodenum);
				kmailbox_write(outbox, &__mailbox_args, MAILBOX_MSG_SIZE);
				kmailbox_close(outbox);

				nodenum += (PROCESSOR_NOC_CNODES_NUM / PROCESSOR_CCLUSTERS_NUM);
			}

			_argc    = 2;
			_argv[0] = (char *) argv[1];
			_argv[1] = (char *) argv[2];
		}
		else
		{
			int inbox;

			inbox = stdinbox_get();

			kprintf("    mailbox BEFORE %d", __mailbox_args.nclusters);
				kmailbox_read(inbox, &__mailbox_args, MAILBOX_MSG_SIZE);
			kprintf("    mailbox AFTER %d", __mailbox_args.nclusters);

			__itoa(__mailbox_args.nclusters, default_argv[0], 10);
			__itoa(__mailbox_args.message_size, default_argv[1], 10);
			kprintf("                  ATOA %s", default_argv[0]);

			_argc    = 2;
			_argv[0] = default_argv[0];
			_argv[1] = default_argv[1];
		}

		fence();

	kprintf(" crt0 EXIT");
	__stdmailbox_cleanup();
	fence_cleanup();

	ret = main(_argc, (const char **) _argv);

	/* Power off. */
	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		___nanvix_exit(ret);

	/* Loop forever. */
	UNREACHABLE();
}
