cd /usr/src/minix/kernel; make && make install
cd /usr/src/minix/lib/libsys; make && make install
cd /usr/src/minix/servers/sched; make && make install
cd /usr/src/minix/servers/pm; make && make install
cd /usr/src/lib/libc; make && make install
cd /usr/src/releasetools; make do-hdboot