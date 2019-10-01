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

#===============================================================================
# Generic Rules
#===============================================================================

# Builds Binary Files
all: all-pingpong all-broadcast all-gather all-allgather

# Cleans Object Files
clean: clean-pingpong clean-broadcast clean-gather clean-allgather

# Builds Binary Image Files
images: image-pingpong image-broadcast image-gather image-allgather

#===============================================================================
# mailbox-pingpong
#===============================================================================

# Builds args.
all-pingpong:
	@$(MAKE) -C pingpong all

# Cleans object files.
clean-pingpong:
	@$(MAKE) -C pingpong clean

# Build Buffer Benchmark Binary Image
image-pingpong:
	bash $(TOOLSDIR)/nanvix-build-image.sh $(IMGDIR)/mailbox-pingpong.img $(BINDIR) mailbox-pingpong

#===============================================================================
# mailbox-broadcast
#===============================================================================

# Builds args.
all-broadcast:
	@$(MAKE) -C broadcast all

# Cleans object files.
clean-broadcast:
	@$(MAKE) -C broadcast clean

# Build Buffer Benchmark Binary Image
image-broadcast:
	bash $(TOOLSDIR)/nanvix-build-image.sh $(IMGDIR)/mailbox-broadcast.img $(BINDIR) mailbox-broadcast


#===============================================================================
# mailbox-gather
#===============================================================================

# Builds args.
all-gather:
	@$(MAKE) -C gather all

# Cleans object files.
clean-gather:
	@$(MAKE) -C gather clean

# Build Buffer Benchmark Binary Image
image-gather:
	bash $(TOOLSDIR)/nanvix-build-image.sh $(IMGDIR)/mailbox-gather.img $(BINDIR) mailbox-gather


#===============================================================================
# mailbox-allgather
#===============================================================================

# Builds args.
all-allgather:
	@$(MAKE) -C allgather all

# Cleans object files.
clean-allgather:
	@$(MAKE) -C allgather clean

# Build Buffer Benchmark Binary Image
image-allgather:
	bash $(TOOLSDIR)/nanvix-build-image.sh $(IMGDIR)/mailbox-allgather.img $(BINDIR) mailbox-allgather
