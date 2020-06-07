#ifndef _S_I_HELLO_H
#define _S_I_HELLO_H

#define HELLO_LEN 64

#define HIOCSETMSG  _IOW('h', 1, char[HELLO_LEN])
#define HIOCGETMSG  _IOR('h', 2, char[HELLO_LEN])

#endif /* _S_I_HELLO_H */