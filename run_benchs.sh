#
# MIT License
#
# Copyright(c) 2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

RAW_FOLDER=raw
LOG_FOLDER=log

mkdir $RAW_FOLDER
mkdir $LOG_FOLDER

# Default configuration
export TARGET=mppa256
export RELEASE=true

KB=1024
sizes="$((4 * KB)) $((8 * KB)) $((16 * KB)) $((32 * KB)) $((64 * KB))"

# Compile
make contrib && make all

# Portal benchmarks
for kernel in broadcast allgather gather pingpong
do
	for size in ${sizes}
	do
		echo "Saving $output/portal-$kernel.csv (ios:1, computes:16, size: $size)"
		make KERNEL=portal-$kernel ARGS="\"1 16 $size\"" run \
		| tee                                                                    \
			>(cat >> $LOG_FOLDER/portal-$kernel.log)                             \
			>(grep -E "portal;" | sed -n -e 's/IODDR0@0.0: RM 1: //p' >> $RAW_FOLDER/portal-$kernel.raw)
	done
done

# Mailbox benchmarks
for kernel in broadcast allgather gather pingpong
do
	for ncclusters in {1..16}
	do
		echo "Saving $output/mailbox-$kernel.csv (ios:1, computes:$ncclusters)"
		make KERNEL=mailbox-$kernel ARGS="\"1 $ncclusters 120\"" run \
		| tee                                                        \
			>(cat >> $LOG_FOLDER/mailbox-$kernel.log)                \
			>(grep -E "mailbox;" | sed -n -e 's/IODDR0@0.0: RM 1: //p' >> $RAW_FOLDER/mailbox-$kernel.raw)
	done
done