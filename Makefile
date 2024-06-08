CROSS_COMPILE		?= 
TARGET				?= bt_start_sniffer
SOURCE_ROOT 		?= ./

CC 					:= $(CROSS_COMPILE)gcc
LD 					:= $(CROSS_COMPILE)ld
OBJCOPY 			:= $(CROSS_COMPILE)objcopy
OBJDUMP				:= $(CROSS_COMPILE)objdump

# Header file directory
INCUDIRS		:= 				${SOURCE_ROOT} \
								${SOURCE_ROOT}/component/config \
								${SOURCE_ROOT}/component/log \
								${SOURCE_ROOT}/component/type \
								${SOURCE_ROOT}/component/zmq \
								${SOURCE_ROOT}/component/ringbuffer \
								${SOURCE_ROOT}/component/controller \
								${SOURCE_ROOT}/component/injection \
				 				

# C source code file directory			
SRCDIRS			:= 				${SOURCE_ROOT} \
								${SOURCE_ROOT}/component/zmq \
								${SOURCE_ROOT}/component/ringbuffer \
								${SOURCE_ROOT}/component/controller \
								${SOURCE_ROOT}/component/injection \
				 				


INCLUDE 		:= 	$(patsubst %, -I %, $(INCUDIRS))


CFILES			:= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))


CFILENDIR		:= $(notdir $(CFILES))

OBJS			:= $(patsubst %, obj/%, $(CFILENDIR:.c=.o))

VPATH			:= $(SRCDIRS)

#CFLAGS 	?= -g -Wall -Werror $(INCLUDE)
CFLAGS 	?= -g -Wall $(INCLUDE)
LDFLAGS ?= -lpthread -lzmq

.PHONY:clean

$(TARGET) : $(CFILES)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@


clean:
	rm -rf $(TARGET) $(OBJS)

	

