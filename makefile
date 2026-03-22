build:
	gcc -c -Iext/inc src/main.c -o main.o
	gcc -c -Iext/inc src/render/render.c -o render.o
	gcc -c -Iext/inc src/assets/assets.c -o assets.o
	ld -m i386pep --subsystem=10 -nostdlib -e efi_main main.o render.o assets.o -o root/efi/boot/bootx64.efi 
	rm main.o
	rm render.o
	rm assets.o

run:
	qemu-system-x86_64 															\
  	-drive if=pflash,format=raw,file="C:/Program Files/qemu/edk2-ovmf/OVMF.fd"  \
  	-drive format=raw,file=fat:rw:root 											\
  	-net none																	
