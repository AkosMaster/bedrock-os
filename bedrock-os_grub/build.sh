make clean
make

if grub-file --is-x86-multiboot os-image.bin; then
  echo multiboot confirmed
else
  echo the file is not multiboot
fi

cp os-image.bin isodir/boot/os-image.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o os-image.iso isodir

genext2fs -d root fs.img -b 1440

make clean
