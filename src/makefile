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
all: all-args all-portal-pingpong # all-fork-join all-gf all-mm all-perf all-stream all-upcall 

# Cleans Object Files
clean: clean-fork-join clean-gf clean-mm clean-perf clean-stream clean-upcall clean-args clean-portal-pingpong

# Builds Binary Image Files
images: image-fork-join image-gf image-mm image-perf image-stream image-upcall image-args image-portal-pingpong

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

#===============================================================================
# fork-join
#===============================================================================

# Builds fork-join.
all-fork-join:
	@$(MAKE) -C fork-join all

# Cleans object files.
clean-fork-join:
	@$(MAKE) -C fork-join clean

# Build Buffer Benchmark Binary Image
image-fork-join:
	bash $(TOOLSDIR)/nanvix-build-image.sh $(IMGDIR)/fork-join.img $(BINDIR) fork-join

#===============================================================================
# gf
#===============================================================================

# Builds gf.
all-gf:
	@$(MAKE) -C gf all

# Cleans object files.
clean-gf:
	@$(MAKE) -C gf clean

# Build Buffer Benchmark Binary Image
image-gf:
	bash $(TOOLSDIR)/nanvix-build-image.sh $(IMGDIR)/gf.img $(BINDIR) gf

#===============================================================================
# mm
#===============================================================================

# Builds mm.
all-mm:
	@$(MAKE) -C mm all

# Cleans object files.
clean-mm:
	@$(MAKE) -C mm clean

# Build Buffer Benchmark Binary Image
image-mm:
	bash $(TOOLSDIR)/nanvix-build-image.sh $(IMGDIR)/mm.img $(BINDIR) mm

#===============================================================================
# perf
#===============================================================================

# Builds perf.
all-perf:
	@$(MAKE) -C perf all

# Cleans object files.
clean-perf:
	@$(MAKE) -C perf clean

# Build Buffer Benchmark Binary Image
image-perf:
	bash $(TOOLSDIR)/nanvix-build-image.sh $(IMGDIR)/perf.img $(BINDIR) perf

#===============================================================================
# stream
#===============================================================================

# Builds stream.
all-stream:
	@$(MAKE) -C stream all

# Cleans object files.
clean-stream:
	@$(MAKE) -C stream clean

# Build Buffer Benchmark Binary Image
image-stream:
	bash $(TOOLSDIR)/nanvix-build-image.sh $(IMGDIR)/stream.img $(BINDIR) stream

#===============================================================================
# upcall
#===============================================================================

# Builds upcall.
all-upcall:
	@$(MAKE) -C upcall all

# Cleans object files.
clean-upcall:
	@$(MAKE) -C upcall clean

# Build Buffer Benchmark Binary Image
image-upcall:
	bash $(TOOLSDIR)/nanvix-build-image.sh $(IMGDIR)/upcall.img $(BINDIR) upcall
