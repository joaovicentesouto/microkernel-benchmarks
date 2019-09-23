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

#include <nanvix/sys/sync.h>

/**
 * @brief Kernel standard sync.
 */
static int __syncin = -1;
static int __syncout = -1;

/**
 * @todo TODO: provide a detailed description for this function.
 */
int fence_setup(int nclusters)
{
	int curr_nodenum;
	int nodes[PROCESSOR_CLUSTERS_NUM];

	curr_nodenum = 0;
	for (int i = 0; i < PROCESSOR_IOCLUSTERS_NUM && i < nclusters; i++)
	{
		nodes[i] = curr_nodenum;
		curr_nodenum += (PROCESSOR_NOC_IONODES_NUM / PROCESSOR_IOCLUSTERS_NUM);
	}
	
	for (
        int i = PROCESSOR_IOCLUSTERS_NUM;
        i < (PROCESSOR_IOCLUSTERS_NUM + PROCESSOR_CCLUSTERS_NUM) && i < nclusters;
        i++
    )
	{
		nodes[i] = curr_nodenum;
		curr_nodenum += (PROCESSOR_NOC_CNODES_NUM / PROCESSOR_CCLUSTERS_NUM);
	}

	/* Master cluster */
	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
    {
        __syncin  = ksync_create(nodes, PROCESSOR_CLUSTERS_NUM, SYNC_ALL_TO_ONE);
        __syncout = ksync_open(nodes, PROCESSOR_CLUSTERS_NUM, SYNC_ONE_TO_ALL);

		return (0);
    }

	/* Slave cluster. */
    __syncout = ksync_open(nodes, PROCESSOR_CLUSTERS_NUM, SYNC_ALL_TO_ONE);
    __syncin  = ksync_create(nodes, PROCESSOR_CLUSTERS_NUM, SYNC_ONE_TO_ALL);

	return (0);
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
int fence_cleanup(void)
{
    ksync_unlink(__syncin);
	return (ksync_close(__syncout));
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
int fence(void)
{
    int ret;

	/* Master cluster */
	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
    {
		ret = ksync_wait(__syncin);
        ret = ksync_signal(__syncout);
    }
    else
    {
        ret = ksync_signal(__syncout);
        ret = ksync_wait(__syncin);
    }

    return (ret);
}
