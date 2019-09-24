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
# Toolchain Configuration
#===============================================================================

# Libraries
export LIBS  = -Wl,--whole-archive $(LIBNANVIX) $(LIBKERNEL) $(LIBHAL)
export LIBS += -Wl,--no-whole-archive $(THEIR_LIBS)

#===============================================================================
# Generic Rules
#===============================================================================

# Builds Binary Files
all: all-args all-portal-pingpong

# Cleans Object Files
clean: clean-args clean-portal-pingpong

# Builds Binary Image Files
images: image-args image-portal-pingpong

#===============================================================================
# args
#===============================================================================

# Builds args.
all-args:
	@$(MAKE) -C args all

# Cleans object files.
clean-args:
	@$(MAKE) -C args clean

# Build Buffer Benchmark Binary Image
image-args:
	bash $(TOOLSDIR)/nanvix-build-image.sh $(IMGDIR)/args.img $(BINDIR) args

#===============================================================================
# portal-pingpong
#===============================================================================

# Builds args.
all-portal-pingpong:
	@$(MAKE) -C portal-pingpong all

# Cleans object files.
clean-portal-pingpong:
	@$(MAKE) -C portal-pingpong clean

# Build Buffer Benchmark Binary Image
image-portal-pingpong:
	bash $(TOOLSDIR)/nanvix-build-image.sh $(IMGDIR)/portal-pingpong.img $(BINDIR) portal-pingpong
