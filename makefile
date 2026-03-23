build:
	g++ -c -std=c++11 -Iext/inc src/main.cpp -o main.o
	g++ -c -std=c++11 -Iext/inc src/render/render.cpp -o render.o
	g++ -c -std=c++11 -Iext/inc src/assets/assets.cpp -o assets.o
	ld -m i386pep --subsystem=10 -nostdlib -e efi_main main.o render.o assets.o -o root/efi/boot/bootx64.efi 
	rm main.o
	rm render.o
	rm assets.o

run:
	qemu-system-x86_64 															\
  	-drive if=pflash,format=raw,file="C:/Program Files/qemu/edk2-ovmf/OVMF.fd"  \
  	-drive format=raw,file=fat:rw:root 											\
  	-net none																	
