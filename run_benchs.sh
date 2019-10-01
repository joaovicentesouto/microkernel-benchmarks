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

output=$1

# Default configuration
export TARGET=mppa256
export RELEASE=true

KB=1024
sizes="$((4 * KB)) $((8 * KB)) $((16 * KB)) $((32 * KB)) $((64 * KB))"

# # Compile
make contrib && make all

# Portal benchmarks
## Runs Ping pong
for size in ${sizes}
do
	make KERNEL=portal-pingpong ARGS="\"1 1 $size\"" run > $output
done

for bench in allgather broadcast gather sather
do
	for niocluster in 1 2
	do
		for nccluster in {1..16}
		do
			for size in ${sizes}
			do
				make KERNEL=portal-$bench ARGS="\"$niocluster $nccluster $size\"" run | grep -E "portal;" >> $output
			done
		done
	done
done


# Mailbox benchmarks
## Runs Ping pong
make KERNEL=mailbox-pingpong ARGS="\"1 1 120\"" run >> $output

for bench in broadcast # allgather need check {sather or gather missing}
do
	for niocluster in 1 2
	do
		for nccluster in {1..16}
		do
			make KERNEL=mailbox-$bench ARGS="\"$niocluster $nccluster 120\"" run | grep -E "portal;" >> $output
		done
	done
done