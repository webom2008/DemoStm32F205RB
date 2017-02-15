# general Makefile

include Makefile.common
LDFLAGS=$(COMMONFLAGS) -fno-exceptions -ffunction-sections -fdata-sections -L$(LIBDIR) -nostartfiles -Wl,--gc-sections,-Tlinker.ld

LDLIBS+=-lm
LDLIBS+=-lstm32

ifeq ($(STM_USB_DEVICE_LIB_USED), ON)
LDLIBS+=-lstm32usbdevice
endif
ifeq ($(STM_USB_HOST_LIB_USED), ON)
LDLIBS+=-lstm32usbhost
endif
ifeq ($(STM_USB_OTG_LIB_USED), ON)
LDLIBS+=-lstm32usbotg
endif

STARTUP=startup.c

all: libs src
	$(CC) -o $(PROGRAM).elf $(LDFLAGS) \
		-Wl,--whole-archive \
			src/app.a \
		-Wl,--no-whole-archive \
			$(LDLIBS)
	$(OBJCOPY) -O ihex $(PROGRAM).elf $(PROGRAM).hex
	$(OBJCOPY) -O binary $(PROGRAM).elf $(PROGRAM).bin

#Extract info contained in ELF to readable text-files:
	arm-none-eabi-readelf -a $(PROGRAM).elf > $(PROGRAM).info_elf
	arm-none-eabi-size -d -B -t $(PROGRAM).elf > $(PROGRAM).info_size
	arm-none-eabi-objdump -S $(PROGRAM).elf > $(PROGRAM).info_code
	arm-none-eabi-nm -t x -S --numeric-sort -s $(PROGRAM).elf > $(PROGRAM).info_symbol

.PHONY: libs src clean tshow

libs:
	$(MAKE) -C libs $@
src:
	$(MAKE) -C src $@

clean:
	$(MAKE) -C libs $@
	$(MAKE) -C src $@
	rm -f $(PROGRAM).elf $(PROGRAM).hex $(PROGRAM).bin $(PROGRAM).info_elf $(PROGRAM).info_size
	rm -f $(PROGRAM).info_code
	rm -f $(PROGRAM).info_symbol

tshow:
	@echo "###################################################################"
	@echo "######## optimize settings: $(InfoTextLib)"
	@echo "######## optimize settings: $(InfoTextSrc)"
	@echo "###################################################################"
#flash:
# ./jtagprog.pl
