/**
 *
 * Phantom OS Unix Box
 *
 * Copyright (C) 2005-2009 Dmitry Zavalishin, dz@dz.ru
 *
 * Network syscalls
 *
 *
**/

#if HAVE_UNIX


#define DEBUG_MSG_PREFIX "Unix/net"
#include "debug_ext.h"
#define debug_level_flow 6
#define debug_level_error 10
#define debug_level_info 10

#include "net.h"
#include "tcp.h"
#include "udp.h"

//#include <unix/uufile.h>
#include <unix/uuprocess.h>
#include <unix/uunet.h>
#include <kernel/unix.h>
#include <sys/unistd.h>
//#include <sys/fcntl.h>
#include <sys/socket.h>
#include <phantom_types.h>

#include <string.h>




static char hostname[MAX_UU_HOSTNAME+1] = "localhost";

// TODO access from object world

int usys_gethostname( int *err, uuprocess_t *u, char *data, size_t len )
{
	(void) u;

    if( len < (unsigned)strlen(hostname) )
    {
        *err = ENAMETOOLONG;
        return -1;
    }

    strcpy( data, hostname );
    return 0;
}


int usys_sethostname( int *err, uuprocess_t *u, const char *data, size_t len )
{
	(void) u;

    if( len > MAX_UU_HOSTNAME )
    {
        *err = ENAMETOOLONG;
        return -1;
    }

    strncpy( hostname, data, len );
    hostname[len] = 0;
    return 0;
}




int usys_socket(int *err, uuprocess_t *u, int domain, int type, int protocol)
{
    if( domain != PF_INET )
    {
        *err = EAFNOSUPPORT;
        return -1;
    }

    if( type != SOCK_STREAM && type != SOCK_DGRAM )
    {
        *err = EINVAL;
        return -1;
    }

    if( protocol != 0 )
        SHOW_INFO( 0, "Warning: proto %d", protocol );

    int isTCP = (type == SOCK_STREAM);

    struct uusocket *us = calloc(1, sizeof(struct uusocket));
    if(us == 0)
    {
        *err = ENOMEM;
        return -1;
    }


    if( isTCP )
    {
        if( tcp_open(&(us->prot_data)) )
        {
            SHOW_ERROR0(0, "can't prepare TCP endpoint");
        fail:
            free(us);
            return 0;
        }
    }
    else
    {
        if( udp_open(&(us->prot_data)) )
        {
            SHOW_ERROR0(0, "can't prepare UDP endpoint");
            goto fail;
        }
    }

    uufile_t *f = create_uufile();

    f->ops = &tcpfs_fops;

    f->pos = 0;
    f->fs = isTCP ? &tcp_fs : &udp_fs;
    f->impl = us;
    f->flags = UU_FILE_FLAG_NET|UU_FILE_FLAG_TCP;


    int fd = uu_find_fd( u, f );

    if( fd < 0 )
    {
        f->fs->close( f );
        *err = EMFILE;
        return -1;
    }

    return fd;
}


int usys_bind(int *err, uuprocess_t *u, int fd, const struct sockaddr *my_addr, socklen_t addrlen)
{
	(void)addrlen;
	(void)my_addr;

    CHECK_FD(fd);
    struct uufile *f = GETF(fd);

    struct uusocket *us = f->impl;

    if( (u == 0) || ! (f->flags & UU_FILE_FLAG_NET))
    {
        *err = ENOTSOCK;
        return -1;
    }

    //us->addr = *my_addr;

    int pe;

    if( f->flags & UU_FILE_FLAG_TCP )
    {
        pe = tcp_bind(us->prot_data, &us->addr);
    }
    else if( f->flags & UU_FILE_FLAG_UDP )
    {
        pe = udp_bind(us->prot_data, &us->addr);

    }
    else
    {
        *err = ENOPROTOOPT;
        return -1;
    }

    // TODO translate!
    if( pe )
        *err = EADDRINUSE;

    return *err ? -1 : 0;
}



#if 0
int usys_accept(int *err, uuprocess_t *u, int fd, const struct sockaddr *acc_addr, socklen_t *addrlen)
{
    CHECK_FD(fd);
    struct uufile *f = GETF(fd);

    struct uusocket *us = f->impl;

    if( (u == 0) || ! (f->flags & (UU_FILE_FLAG_NET|UU_FILE_FLAG_TCP)))
    {
        *err = ENOTSOCK;
        return -1;
    }

    //us->addr = my_addr;

    int pe = tcp_accept(us->prot_data, tmp_addr);

    // TODO check len
    acc_addr = tmp_addr;

    // TODO translate!
    if( pe )
    {
        *err = ECONNABORTED;
        return -1;
    }

#error TODO impl

    return *err ? -1 : 0;
}
#endif








int usys_listen(int *err, uuprocess_t *u, int fd, int backlog)
{
	(void)backlog;

    CHECK_FD(fd);
    struct uufile *f = GETF(fd);

    struct uusocket *us = f->impl;

    if( (u == 0) || ! (f->flags & (UU_FILE_FLAG_NET|UU_FILE_FLAG_TCP)))
    {
        *err = ENOTSOCK;
        return -1;
    }

    if( tcp_listen(us->prot_data) )
        *err = EISCONN;

    return *err ? -1 : 0;
}





int usys_getsockopt(int *err, uuprocess_t *u, int fd, int level, int optname, void *optval, socklen_t *optlen)
{
	(void) optname;
	(void) optlen;
	(void) optval;

    CHECK_FD(fd);
    struct uufile *f = GETF(fd);

    struct uusocket *us = f->impl;
	(void) us;


    if( (u == 0) || ! (f->flags & UU_FILE_FLAG_NET))
    {
        *err = ENOTSOCK;
        return -1;
    }

    if( level != SOL_SOCKET )
    {
        *err = ENOPROTOOPT;
        return -1;
    }


    *err = ENOPROTOOPT;
    return *err ? -1 : 0;
}

int usys_setsockopt(int *err, uuprocess_t *u, int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    CHECK_FD(fd);
    struct uufile *f = GETF(fd);

    struct uusocket *us = f->impl;

    if( (u == 0) || ! (f->flags & UU_FILE_FLAG_NET))
    {
        *err = ENOTSOCK;
        return -1;
    }

    if( level != SOL_SOCKET )
    {
        *err = ENOPROTOOPT;
        return -1;
    }

    if( (optname > 0x1000) && (optname < 0x100F) )
    {
        switch(optname)
        {
        }
    }
    else
    {
        // Flags, one bit

        if(optlen != sizeof(int))
        {
            *err = ENOPROTOOPT;
            return -1;
        }
        int set = *(int *)optval;

        if( set )
            us->options |= optname;
        else
            us->options &= ~optname;

        // TODO this 'us->options' is not connected to something in IP stack!
        // so return err yet
        //return 0;
    }



    *err = ENOPROTOOPT;
    return *err ? -1 : 0;
}


int usys_getsockname(int *err, uuprocess_t *u, int fd, struct sockaddr *name, socklen_t *namelen)
{
    CHECK_FD(fd);
    struct uufile *f = GETF(fd);

    struct uusocket *us = f->impl;

    if( (u == 0) || ! (f->flags & UU_FILE_FLAG_NET))
    {
        *err = ENOTSOCK;
        return -1;
    }

    if( (unsigned)(*namelen) < sizeof(us->addr) )
    {
        *err = EINVAL;
        return -1;
    }

    *namelen = sizeof(us->addr);
    *name = us->addr;
    return 0;
}

int usys_getpeername(int *err, uuprocess_t *u, int fd, struct sockaddr *name, socklen_t *namelen)
{
    CHECK_FD(fd);
    struct uufile *f = GETF(fd);

    struct uusocket *us = f->impl;

    if( (u == 0) || ! (f->flags & UU_FILE_FLAG_NET))
    {
        *err = ENOTSOCK;
        return -1;
    }

    if( ! (f->flags & UU_FILE_FLAG_TCP) )
    {
        *err = ENOTCONN; // Right?
        return -1;
    }

    if( (unsigned)(*namelen) < sizeof(struct sockaddr) )
    {
        *err = EINVAL;
        return -1;
    }

    *namelen = sizeof(struct sockaddr);

    if( tcp_getpeername(us->prot_data, name) )
    {
        *err = ENOTCONN;
        return -1;
    }

    return 0;
}










ssize_t usys_recv(int *err, uuprocess_t *u, int fd, void *buf, size_t buflen, int flags)
{
    return usys_recvfrom(err, u, fd, buf, buflen, flags, 0, 0 );
}

ssize_t usys_recvfrom(int *err, uuprocess_t *u, int fd, void *buf, size_t buflen, int flags,
                      struct sockaddr *from, socklen_t *fromlen)
{
	(void) fromlen;
	(void) from;
	(void) buflen;
	(void) buf;

    CHECK_FD(fd);
    struct uufile *f = GETF(fd);
    int len = 0;

    struct uusocket *us = f->impl;
	(void) us;


    if( (u == 0) || ! (f->flags & UU_FILE_FLAG_NET))
    {
        *err = ENOTSOCK;
        return -1;
    }

    if( flags )
        SHOW_ERROR( 0, "I don't know this flag %d", flags );



    *err = ENOSYS;
    return *err ? -1 : len;
}

ssize_t usys_recvmsg(int *err, uuprocess_t *u, int fd, struct msghdr *msg, int flags)
{
	(void) msg;

	CHECK_FD(fd);
    struct uufile *f = GETF(fd);
    int len = 0;

    struct uusocket *us = f->impl;
	(void) us;


    if( (u == 0) || ! (f->flags & UU_FILE_FLAG_NET))
    {
        *err = ENOTSOCK;
        return -1;
    }


    if( flags )
        SHOW_ERROR( 0, "I don't know this flag %d", flags );



    *err = ENOSYS;
    return *err ? -1 : len;
}






ssize_t usys_send(int *err, uuprocess_t *u, int fd, const void *buf, size_t len, int flags)
{
    return usys_sendto(err, u, fd, buf, len, flags, 0, 0 );
}

ssize_t usys_sendto(int *err, uuprocess_t *u, int fd, const void *buf, size_t buflen, int flags, const struct sockaddr *to, socklen_t tolen)
{
	(void) tolen;
	(void) to;
	(void) buflen;
	(void) buf;

    CHECK_FD(fd);
    struct uufile *f = GETF(fd);
    int len = 0;

    struct uusocket *us = f->impl;
	(void) us;

    if( (u == 0) || ! (f->flags & UU_FILE_FLAG_NET))
    {
        *err = ENOTSOCK;
        return -1;
    }


    if( flags )
        SHOW_ERROR( 0, "I don't know this flag %d", flags );



    *err = ENOSYS;
    return *err ? -1 : len;
}

ssize_t usys_sendmsg(int *err, uuprocess_t *u, int fd, const struct msghdr *msg, int flags)
{
	(void) msg;

	CHECK_FD(fd);
    struct uufile *f = GETF(fd);
    int len = 0;

    struct uusocket *us = f->impl;
	(void) us;


    if( (u == 0) || ! (f->flags & UU_FILE_FLAG_NET))
    {
        *err = ENOTSOCK;
        return -1;
    }

    if( flags )
        SHOW_ERROR( 0, "I don't know this flag %d", flags );




    *err = ENOSYS;
    return *err ? -1 : len;
}




#endif // HAVE_UNIX

