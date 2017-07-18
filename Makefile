MONO_PATH=$(subst \,/,$(shell monomake path --bare))
include $(MONO_PATH)/predefines.mk

TARGET=temperature

include $(MONO_PATH)/mono.mk

-include Makefile.local
