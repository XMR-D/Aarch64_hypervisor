include mkflags.mk

include arch/$(ARCH)/Makefile
include hypervisor/Makefile
include  plat/$(PLAT)/Makefile

$(srcs-y):=

objs-y:=$(patsubst %.c,%.c.o,$(srcs-y))
objs-y:=$(patsubst %.S,%.S.o,$(objs-y))

.PHONY: all
all: build

build: pflash.bin

run: build
	sudo qemu-system-$(ARCH) -machine $(PLAT) -cpu cortex-a72 -nographic -kernel pflash.bin -serial mon:stdio -m 2G -smp 4

screen: build
	qemu-system-$(ARCH) -machine $(PLAT) -cpu cortex-a72 -nographic -kernel pflash.bin -serial pty -m 2G -smp 4

debug: build
	qemu-system-$(ARCH) -machine $(PLAT),dumpdtb=Hypervisor.dtb -cpu cortex-a72 -nographic -kernel pflash.bin -singlestep -d in_asm -D DebugTrace.txt -serial mon:stdio -m 2G -smp 4
	sudo qemu-system-$(ARCH) -machine $(PLAT) -cpu cortex-a72 -nographic -serial file:/dev/tty3 -kernel pflash.bin -singlestep -d in_asm -D DebugTrace.txt -serial mon:stdio -m 2G -smp 4
	$(ARCH)-linux-gnu-objdump -D -h hypervisor.elf > HypervisorDump.txt


%.c.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@


%.S.o: %.S
	$(AS) $(INCLUDES) $(SFLAGS) $^ -o $@

%.ld: %.lds
	$(AS) $^ -o $@

.PRECIOUS: %.elf
hypervisor.elf: $(objs-y)
	$(LD) $(LDFLAGS) $^ -o $@

.PRECIOUS: %.dump
%.dump: %.elf
	$(V)$(OBJDUMP) -D $^ > $@

.PRECIOUS: %.img
hypervisor.img: hypervisor.elf
	$(OBJCOPY) -O binary $< $@

.PRECIOUS: pflash.bin
pflash.bin: hypervisor.img
	$(V)dd if=/dev/zero of=$@ bs=1M count=512
	$(V)dd if=$< of=$@ conv=notrunc bs=1M count=20
#$(V)dd if=$(KERNEL) of=$@ conv=notrunc bs=1M seek=50 (used to copy kernel images into the final binary)

clean:
	rm -rf *.img *.elf *.bin *.dump *.o $(MAIN)/*.o plat/$(PLAT)/*.o arch/$(ARCH)/*.o HypervisorDump.txt DebugTrace.txt Hypervisor.dtb
