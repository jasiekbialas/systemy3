cd /usr/src/minix/fs/mfs; make clean && make && make install
umount /root/nowy
mount /dev/c0d1 /root/nowy
