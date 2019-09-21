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
#include <nanvix/sys/sync.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/portal.h>
#include <stddef.h>
#include <kbench.h>

struct __mailbox_args
{
	int nclusters;
	char unused[(MAILBOX_MSG_SIZE - sizeof(int))];
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

/**
 *  Convert a string to a int.
 */
int __atoi(const char * str)
{
	int result;

	result = 0;

	for (int i = 0; str[i] != '\0'; ++i)
		result = (result * 10) + str[i] - '0';

	return result;
}

/* A utility function to reverse a string  */
void reverse(char str[], int length)
{
	char swap;
	int start = 0;
	int end = length -1;
	while (start < end)
	{
		swap = *(str+start);
		*(str+start) = *(str+end);
		*(str+end) = swap;
		start++;
		end--;
	}
}

/**
 *  Convert a int to a string.
 */
char* __itoa(int num, char* str, int base)
{
	int i = 0;
	bool isNegative = false;

	/* Handle 0 explicitely, otherwise empty string is printed for 0 */
	if (num == 0)
	{
		str[i++] = '0';
		str[i] = '\0';
		return str;
	}

	// In standard itoa(), negative numbers are handled only with
	// base 10. Otherwise numbers are considered unsigned.
	if (num < 0 && base == 10)
	{
		isNegative = true;
		num = -num;
	}

	// Process individual digits
	while (num != 0)
	{
		int rem = num % base;
		str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
		num = num/base;
	}

	// If number is negative, append '-'
	if (isNegative)
		str[i++] = '-';

	str[i] = '\0'; // Append string terminator

	reverse(str, i);

	return str;
}

/*
 * Entry point of the program.
 */
void ___start(int argc, const char *argv[], char **envp)
{
	int ret;
	int _argc;
	char *_argv[1];
	char default_argv[1][256];
	struct __mailbox_args __mailbox_args;

	UNUSED(argc);
	UNUSED(envp);

	__mailbox_args.nclusters = 0;

	kprintf(" crt0 SETUP");
	__stdsync_setup();
	__stdmailbox_setup();

		if (cluster_get_num() != PROCESSOR_CLUSTERNUM_MASTER)
		{
			int dummy = 20;
			while (dummy--)
				kprintf("    dummy");
		}

		kprintf(" crt0 FENCE");
		stdsync_fence();

		if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		{
			int outbox;
			int nodenum;

			nodenum = 0;
			__mailbox_args.nclusters = __atoi(argv[1]);

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

			_argc    = 1;
			_argv[0] = (char *) argv[1];
		}
		else
		{
			int inbox;

			inbox = stdinbox_get();

			kprintf("    mailbox BEFORE %d", __mailbox_args.nclusters);
				kmailbox_read(inbox, &__mailbox_args, MAILBOX_MSG_SIZE);
			kprintf("    mailbox AFTER %d", __mailbox_args.nclusters);

			__itoa(__mailbox_args.nclusters, default_argv[0], 10);
			kprintf("                  ATOA %s", default_argv[0]);

			_argc    = 1;
			_argv[0] = default_argv[0];
		}

	kprintf(" crt0 EXIT");
	__stdmailbox_cleanup();
	__stdsync_cleanup();

	ret = main(_argc, (const char **) _argv);

	/* Power off. */
	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
		___nanvix_exit(ret);

	/* Loop forever. */
	UNREACHABLE();
}
