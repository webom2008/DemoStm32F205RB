# libs Makefile
include ../Makefile.common

LIBS=libstm32.a
ifeq ($(STM_USB_DEVICE_LIB_USED), ON)
LIBS+=libstm32usbdevice.a
endif
ifeq ($(STM_USB_HOST_LIB_USED), ON)
LIBS+=libstm32usbhost.a
endif
ifeq ($(STM_USB_OTG_LIB_USED), ON)
LIBS+=libstm32usbotg.a
endif
ifeq ($(SYS_FREERTOS_USED), ON)
LIBS+=libfreertos.a
endif

CFLAGSlib+=-c
#CFLAGSlib+=-c -fno-strict-aliasing

all: libs

libs: $(LIBS)

libstm32.a:
	@echo -n "Building $@ ..."
	@cd $(STMLIB)/CMSIS/CM3/CoreSupport && \
		$(CC) $(CFLAGSlib) \
			-D"assert_param(expr)=((void)0)" \
			core_cm3.c
	@cd $(STMLIB)/STM32F2xx_StdPeriph_Driver/src && \
		$(CC) $(CFLAGSlib) \
			-D"assert_param(expr)=((void)0)" \
			*.c
	@$(AR) cr $(LIBDIR)/$@ \
		$(STMLIB)/CMSIS/CM3/CoreSupport/core_cm3.o \
		$(STMLIB)/STM32F2xx_StdPeriph_Driver/src/*.o
	@echo "done."
        
libstm32usbdevice.a:
	@echo "Building $@ ..."
	@echo "cd $(STMUSB_DEVICE_LIB)/Core/src"
	@cd $(STMUSB_DEVICE_LIB)/Core/src && \
		$(CC) $(CFLAGSlib) \
			-D"assert_param(expr)=((void)0)" \
			*.c
	@echo "cd $(STMUSB_DEVICE_LIB)/Class/hid/src"
	@cd $(STMUSB_DEVICE_LIB)/Class/hid/src && \
		$(CC) $(CFLAGSlib) \
			-D"assert_param(expr)=((void)0)" \
			*.c
	@$(AR) cr $(LIBDIR)/$@ \
        $(STMUSB_DEVICE_LIB)/Core/src/*.o \
        $(STMUSB_DEVICE_LIB)/Class/hid/src/*.o \
    
	@echo "done."
    
libstm32usbhost.a:
	@echo -n "Building $@ ..."
	@cd $(STMUSB_HOST_LIB)/src && \
		$(CC) $(CFLAGSlib) \
			-D"assert_param(expr)=((void)0)" \
			-I../inc \
			*.c
	@$(AR) cr $(LIBDIR)/$@ $(STMUSB_HOST_LIB)/src/*.o
	@echo "done."
    
libstm32usbotg.a:
	@echo "Building $@ ..."
	@echo "cd $(STMUSB_OTG_LIB)/src"
	@cd $(STMUSB_OTG_LIB)/src && \
		$(CC) $(CFLAGSlib) -fno-strict-aliasing \
			-D"assert_param(expr)=((void)0)" \
			usb_dcd_int.c \
            usb_core.c \
            usb_dcd.c
	@$(AR) cr $(LIBDIR)/$@ $(STMUSB_OTG_LIB)/src/*.o
	@echo "done."

libfreertos.a:
	@echo "Building $@ ..."
	@echo "cd $(FREERTOS_LIB_DIR)"
	@cd $(FREERTOS_LIB_DIR)/portable/GCC/ARM_CM3 && \
		$(CC) $(CFLAGSlib) \
			-D"assert_param(expr)=((void)0)" \
			port.c
	@cd $(FREERTOS_LIB_DIR)/portable/MemMang && \
		$(CC) $(CFLAGSlib) \
			-D"assert_param(expr)=((void)0)" \
			heap_3.c
	@cd $(FREERTOS_LIB_DIR) && \
		$(CC) $(CFLAGSlib) -fno-strict-aliasing \
			-D"assert_param(expr)=((void)0)" \
            list.c \
            queue.c \
            tasks.c
	@$(AR) cr $(LIBDIR)/$@ \
		$(FREERTOS_LIB_DIR)/portable/GCC/ARM_CM3/*.o \
		$(FREERTOS_LIB_DIR)/portable/MemMang/*.o \
		$(FREERTOS_LIB_DIR)/*.o
	@echo "done."

.PHONY: libs clean tshow

clean:
	rm -f $(STMLIB)/CMSIS/Device/ST/STM32F2xx/Source/Templates/system_stm32f2xx.o
	rm -f $(STMLIB)/CMSIS/CM3/CoreSupport/core_cm3.o
	rm -f $(STMLIB)/STM32F2xx_StdPeriph_Driver/src/*.o
	rm -f $(STMUSB_DEVICE_LIB)/Core/src/*.o
	rm -f $(STMUSB_HOST_LIB)/src/*.o
	rm -f $(STMUSB_OTG_LIB)/src/*.o
	rm -f $(FREERTOS_LIB_DIR)/portable/GCC/ARM_CM3/*.o
	rm -f $(FREERTOS_LIB_DIR)/portable/MemMang/*.o
	rm -f $(FREERTOS_LIB_DIR)/*.o
	rm -f $(LIBS)
tshow:
	@echo "##################################################"
	@echo "## optimize settings: $(InfoTextLib)"
	@echo "## optimize settings: $(InfoTextSrc)"
	@echo "##################################################"
