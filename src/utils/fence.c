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
#include <kbench.h>

/**
 * @brief Kernel standard sync.
 */
static int __syncin = -1;
static int __syncout = -1;

/**
 * @todo TODO: provide a detailed description for this function.
 */
int fence_setup(int nioclusters, int ncclusters)
{
	unsigned open_mode;
	unsigned create_mode;
	int nodes[PROCESSOR_CLUSTERS_NUM];

	build_node_list(nioclusters, ncclusters, nodes);

	/* Master cluster */
	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
	{
		create_mode = SYNC_ALL_TO_ONE;
		open_mode   = SYNC_ONE_TO_ALL;
	}
	else
	{
		create_mode = SYNC_ONE_TO_ALL;
		open_mode   = SYNC_ALL_TO_ONE;
	}

	/* Slave cluster. */
	KASSERT((__syncin  = ksync_create(nodes, (nioclusters + ncclusters), create_mode)) >= 0);
	KASSERT((__syncout = ksync_open(nodes, (nioclusters + ncclusters), open_mode)) >= 0);

	return (0);
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
int fence_cleanup(void)
{
	KASSERT(ksync_unlink(__syncin) == 0);
	KASSERT(ksync_close(__syncout) == 0);

	__syncin  = -1;
	__syncout = -1;

	return (0);
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
int fence(void)
{
	kprintf("    + fence");

	/* Master cluster */
	if (cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
	{
		KASSERT(ksync_wait(__syncin) == 0);
		KASSERT(ksync_signal(__syncout) == 0);
	}
	else
	{
		KASSERT(ksync_signal(__syncout) == 0);
		KASSERT(ksync_wait(__syncin) == 0);
	}

	kprintf("    - fence");

	return (0);
}
