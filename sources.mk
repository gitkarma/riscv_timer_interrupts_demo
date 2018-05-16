
SOURCES = $(SRC_DIR)entry.S \
			$(SRC_DIR)main.c \
			$(SRC_DIR)syscalls.c \
			$(SRC_DIR)trap_handler.c

INCLUDES = -I./include


##################################################################
#  SPECIFIC PARAMETERS
##################################################################
PROJi  = proj3
N_PROC ?= 1
ISA ?= rv64imafd
ABI ?= lp64
SPIKE_RBB_PORT = 9824
CEMUL_RBB_PORT = 9823
#SPIKE_SIMULATION ?= -DSPIKE_SIMULATION

# To enable DEBUGGING, set ENABLE_DEBUG to 1
# Other values will set optimization level to 3 by default
ENABLE_DEBUG = 0
