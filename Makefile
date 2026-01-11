include mkflags.mk

#Do not change the order, if plat is compiled before it won't run (TOFIX)
include arch/$(ARCH)/Makefile
include hypervisor/Makefile
include plat/$(PLAT)/Makefile

$(srcs-y):=

objs-y:=$(patsubst %.c,%.c.o,$(srcs-y))
objs-y:=$(patsubst %.S,%.S.o,$(objs-y))

run: build
	$(QEMU_PATH)-$(ARCH) -machine $(BOOTING_MODE) -cpu cortex-a72 -nographic -kernel pflash.bin -serial mon:stdio -m 4G -smp 4

screen: build
	$(QEMU_PATH)-$(ARCH) -machine $(BOOTING_MODE) -cpu cortex-a72 -nographic -kernel pflash.bin -serial pty -m 4G -smp 4

debug: build
	$(QEMU_PATH)-$(ARCH) -machine $(DEBUG_BOOTING_MODE) -cpu cortex-a72 -nographic -kernel pflash.bin -accel tcg,one-insn-per-tb=on -serial mon:stdio -m 2G -smp 4
	$(QEMU_PATH)-$(ARCH) -machine $(BOOTING_MODE) -cpu cortex-a72 -nographic -kernel pflash.bin -accel tcg,one-insn-per-tb=on -d int,mmu,in_asm -D DebugTrace.txt -serial mon:stdio -m 2G -smp 4

build: pflash.bin

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
#create a pflash.bin of 512M full of 0
	$(V)dd if=/dev/zero of=$@ bs=1M count=512
#copy 20M of hypervisor.img into pflash.bin 
	$(V)dd if=$< of=$@ conv=notrunc bs=1M count=20
#copy the VM at 25M into the pflash.bin (tables will be placed before so isok)
	$(V)dd if=$(VM) of=$@ conv=notrunc bs=1M seek=25

clean:
	rm -rf *.img *.elf *.bin *.dump *.o hypervisor/*.o plat/$(PLAT)/*.o arch/$(ARCH)/*.o arch/$(ARCH)/include/ENVCONFS.h HypervisorDump.txt DebugTrace.txt Hypervisor.dtb
