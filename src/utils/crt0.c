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
	int _argc;
	char *_argv[1];
	char default_argv[3][100];
	struct __mailbox_args __mailbox_args;

	UNUSED(argc);
	UNUSED(envp);

	fence_setup(PROCESSOR_IOCLUSTERS_NUM, PROCESSOR_CCLUSTERS_NUM);
	__stdmailbox_setup();

		if (cluster_get_num() != PROCESSOR_CLUSTERNUM_MASTER)
		{
			kprintf("    1. dummy");

				timer(5 * CLUSTER_FREQ);
				timer(CLUSTER_FREQ);

			kprintf("    2. dummy");
		}

		fence();

		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		{
			int outbox;
			int nodenum;

			nodenum = 0;
			__mailbox_args.nioclusters  = __atoi(argv[1]);
			__mailbox_args.ncclusters   = __atoi(argv[2]);
			__mailbox_args.message_size = __atoi(argv[3]);

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

			_argc    = 3;
			_argv[0] = (char *) argv[1];
			_argv[1] = (char *) argv[2];
			_argv[2] = (char *) argv[3];
		}
		else
		{
			int inbox;

			inbox = stdinbox_get();

			kmailbox_read(inbox, &__mailbox_args, MAILBOX_MSG_SIZE);

			__itoa(__mailbox_args.nioclusters,  default_argv[0], 10);
			__itoa(__mailbox_args.ncclusters,   default_argv[1], 10);
			__itoa(__mailbox_args.message_size, default_argv[2], 10);

			_argc    = 3;
			_argv[0] = default_argv[0];
			_argv[1] = default_argv[1];
			_argv[2] = default_argv[2];
		}

		fence();

	__stdmailbox_cleanup();
	fence_cleanup();

	ret = main(_argc, (const char **) _argv);

	/* Power off. */
	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		___nanvix_exit(ret);

	/* Loop forever. */
	UNREACHABLE();
}
