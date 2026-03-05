#
# Copyright (c) 2026, AMD IBS Test Team
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Author: AMD IBS Test Team
# Date Created: 2026-02-22
#

# AMD PMC Test Suite - Makefile

PROG=	pmc_l3_test pmc_df_test pmc_df_zen4_test

SRCS=	pmc_l3_test.c pmc_df_test.c pmc_df_zen4_test.c

# FreeBSD-specific build flags
CFLAGS+= -I${.CURDIR} -DAMD_PMC_TESTS
CFLAGS+= -Wall -Wextra -Werror

# Link with PMC library
LDADD+= -lpmc

# Build targets
all: ${PROG}

# Clean build artifacts
clean:
	rm -f ${PROG} *.o

# Install tests
install:
	${INSTALL_PROGRAM} ${PROG} ${DESTDIR}${BINDIR}/

# Run tests locally
test: all
	@echo "Running PMC Core L3 tests..."
	./pmc_l3_test all
	@echo ""
	@echo "Running PMC Uncore DF tests..."
	./pmc_df_test all
	@echo ""
	@echo "Running PMC Uncore DF Zen4/Zen5 tests..."
	./pmc_df_zen4_test all

.PHONY: all clean install test
