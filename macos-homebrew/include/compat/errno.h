#ifndef ESTRPIPE
#define ESTRPIPE EPIPE   /* macOS has no STREAMS; map to nearest error */
#endif

#ifndef ENONET
#define ENONET 66   /* Linux errno, safe placeholder */
#endif

#ifndef ENOPKG
#define ENOPKG 65    /* Choose any unused value: 65 is used by Linux */
#endif

#ifndef EREMOTEIO
#define EREMOTEIO 121
#endif
#ifndef ERFKILL
#define ERFKILL 132
#endif
#ifndef EBADR
#define EBADR 53
#endif
#ifndef EUNATCH
#define EUNATCH 49
#endif
#ifndef EMEDIUMTYPE
#define EMEDIUMTYPE 124
#endif
#ifndef EKEYREJECTED
#define EKEYREJECTED 129
#endif
#ifndef ENOKEY
#define ENOKEY 126
#endif
#ifndef EUCLEAN
#define EUCLEAN 117
#endif
#ifndef EBADSLT
#define EBADSLT 53
#endif
#ifndef ENOANO
#define ENOANO 55
#endif
#ifndef ENOCSI
#define ENOCSI 50
#endif
#ifndef EKEYREVOKED
#define EKEYREVOKED 128
#endif

#ifndef ENOMEDIUM
#define ENOMEDIUM ENODEV
#endif

#ifndef EBADFD
#define EBADFD EBADF
#endif

#ifndef ELIBBAD
#define ELIBBAD EBADF
#endif

#ifndef EXFULL
#define EXFULL ENOSPC
#endif
