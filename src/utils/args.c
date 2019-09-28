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
#include <nanvix/sys/mailbox.h>
#include <stddef.h>
#include <kbench.h>

struct initial_arguments
{
	int ncclusters;
	int nioclusters;
	int message_size;
	char unused[(MAILBOX_MSG_SIZE - (3 * sizeof(int)))];
};

void send_arguments(struct initial_arguments * args)
{
    int outbox;
    int nodenum;
    int nodes[PROCESSOR_CLUSTERS_NUM];

    build_node_list(PROCESSOR_IOCLUSTERS_NUM, PROCESSOR_CCLUSTERS_NUM, nodes);

    for (int i = 0; i < PROCESSOR_CLUSTERS_NUM; ++i)
    {
        KASSERT((outbox = kmailbox_open(nodes[i])) >= 0);
            KASSERT(kmailbox_write(outbox, args, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
        KASSERT(kmailbox_close(outbox) == 0);
    }
}

void receive_arguments(struct initial_arguments * args)
{
    int inbox;

    __stdmailbox_setup();

        timer(CLUSTER_FREQ);

        KASSERT((inbox = stdinbox_get()) >= 0);
        KASSERT(kmailbox_read(inbox, &args, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);

    __stdmailbox_cleanup();
}
