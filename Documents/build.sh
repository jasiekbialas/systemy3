
cd /usr/src/minix/kernel; make && make install
cd /usr/src/minix/lib/libsys; make && make install
cd /usr/src/minix/servers/sched; make clean && make && make install
cd /usr/src/minix/servers/pm; make clean && make && make install
cd /usr/src/lib/libc; make clean && make && make install
cd /usr/src/releasetools; make do-hdboot