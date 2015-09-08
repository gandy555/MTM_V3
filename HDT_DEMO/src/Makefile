#
# Makefile
#

TARGET	= mts_main

SOURCES	= \
	  common.cpp \
	  timer.cpp \
	  gpio.cpp \
	  gpio_mtm.cpp \
	  watchdog.cpp \
	  microwave.cpp \
	  file.cpp \
	  sysv_mq.cpp \
	  message.cpp \
	  serial.cpp \
	  state.cpp \
	  object.cpp \
	  nano-X_custom.cpp \
	  setup_data.cpp \
	  wallpad_hdt.cpp \
	  wallpad_shn.cpp \
	  wallpad_kcm.cpp \
	  context.cpp \
	  context_light.cpp \
	  context_elevator.cpp \
	  context_gas.cpp \
	  context_parking.cpp \
	  context_security.cpp \
	  context_setup.cpp \
	  context_weather.cpp \
	  main.cpp \

PREFIX	= arm-linux
AR	= $(PREFIX)-ar
CC	= $(PREFIX)-gcc
CP	= $(PREFIX)-g++
LD	= $(PREFIX)-ld

WORK_DIR = /home/freejack/MTS_Work/mtm_v3
NANOX_INC_PATH	= $(WORK_DIR)/nanoX/include
NANOX_LIB_PATH	= $(WORK_DIR)/nanoX/libs

DEFINE	 = -D_ARM_
INCLUDE  += -I../include -I$(NANOX_INC_PATH)
LIBS	 = -L$(NANOX_LIB_PATH) -lnano-X -lfreetype -ljpeg -lpng12 -lz -lm -lpthread -lrt 

OBJ_DIR  = ../obj
BIN_DIR  = ../bin
OBJS	= $(SOURCES:.cpp=.o)

.SUFFIXES : .cpp .o

.cpp.o :
	$(CP) -O2 -c $(DEFINE) $(INCLUDE) $(CFLAGS) -o $(OBJ_DIR)/$@ $<

all : $(TARGET)

$(TARGET): $(OBJS)
	$(CP) $(addprefix $(OBJ_DIR)/, $(OBJS)) $(LIBS) -o $(BIN_DIR)/$(TARGET) 

clean :
	rm -rf $(addprefix $(OBJ_DIR)/, $(OBJS)) $(BIN_DIR)/$(TARGET)
