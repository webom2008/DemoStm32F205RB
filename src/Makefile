# src Makefile

include ../Makefile.common


SRCDIR=$(TOP)/src
#CFLAGS+=-Werror

SRCS=app.a

all: src

src: $(SRCS)

SRC_FILES=startup.c \
		  main.c \
	  		stm32fxxx_it.c \
	  		usb_bsp.c \
	  		usbd_usr.c \
	  		usbd_desc.c \
	  		usart.c \
	  		system_tick.c \
	  		test_keyboard.c \
	  		test_mouse.c \
	  		test_mtouch.c \
	  		buttons.c \
	  		fifo.c \
	  		system_stm32f2xx.c \
	  		stm32_eval.c \
	  		lcd_log.c \
	  		stm322xg_eval_key.c \
	  		LTM022A69B.c

OBJS=startup.o \
	main.o \
	stm32fxxx_it.o \
	usart.o \
	system_tick.o \
	system_stm32f2xx.o \
	fifo.o \
	udprintf.o

app.a:$(OBJS)
	$(AR) cr app.a $(OBJS)

app_test.a:
	@echo "Building $@ ..."
	@echo "cd $(SRCDIR)"
	@cd $(SRCDIR) && \
		$(CC) $(CFLAGS) \
			-D"assert_param(expr)=((void)0)" \
			$(SRC_FILES)
	$(AR) cr $(SRCDIR)/$@ \
		$(SRCDIR)/*o
	@echo "Done."

.PHONY: src clean tshow

clean:
#	@echo $(CFLAGS)
#	@echo $(CFLAGSlib)
	rm -f $(SRCDIR)/*.o
	rm -f $(SRCS)
tshow:
	@echo "##########################################################"
	@echo "######## optimize settings: $(InfoTextLib)"
	@echo "######## optimize settings: $(InfoTextSrc)"
	@echo "##########################################################"
