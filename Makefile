MONO_PATH=/usr/local/openmono
include $(MONO_PATH)/predefines.mk

TARGET=temperature

include $(MONO_PATH)/mono.mk

-include Makefile.local
