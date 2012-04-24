#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include <memdraw.h>
#include <keyboard.h>
#include <mouse.h>
#include <cursor.h>
#include <drawfcall.h>
#include "fcall.h"

static void readproc(void *arg);
static uchar *readmsg(int fd);
static int writemsg(int fd, uchar *mbuf);
static void proxyfcallsproc(void *arg);

Channel *
readfcalls(int fd)
{
	Channel *ch;
	void **args;

	if ((args = malloc(2*sizeof(void*))) == nil)
		sysfatal("malloc: %r");

	ch = chancreate(sizeof(void*), 0);
	args[0] = ch;
	args[1] = (void*)(intptr)fd;
	proccreate(readproc, args, 1<<15);
	
	return ch;
}

// channel carries the error (%r).
Channel *
procproxyfcalls(int readfd, int writefd)
{
	int *fds;

	if ((fds = malloc(2*sizeof(int))) == nil)
		sysfatal("malloc: %r");
	
	fds[0] = readfd;
	fds[1] = writefd;
	proccreate(proxyfcallsproc, fds, 1<<15);
	return nil;
}

void
proxyfcalls(int readfd, int writefd)
{
	int r;
	uchar *mbuf = nil;

	do{
		if((mbuf = readmsg(readfd)) == nil)
			break;
		r = writemsg(writefd, mbuf);
		free(mbuf);
	}while(r >= 0);
}

static void
proxyfcallsproc(void *arg)
{
	int *fds = (int*)arg;
	int writefd, readfd;
	
	readfd = fds[0];
	writefd = fds[1];
	free(arg);
	proxyfcalls(readfd, writefd);
}

static void
readproc(void *arg)
{
	uchar *mbuf;
	int fd;
	Channel *ch;
	void **args = (void **)arg;
	
	ch = (Channel *)args[0];
	fd = (uint)(intptr)args[1];

	while((mbuf = readmsg(fd)) != nil)
		sendp(ch, mbuf);
}

static uchar *
readmsg(int fd)
{
	int n;
	uchar buf[4], *mbuf;

	if(readn(fd, buf, 4) != 4){
		werrstr("eof on length");
		return nil;
	}
	
	GET(buf, n);
	if((mbuf = malloc(n+4)) == nil)
		return nil;

	PUT(mbuf, n);
	if(readn(fd, mbuf+4, n-4) != n-4){
		free(mbuf);
		werrstr("eof on input");
		return nil;
	}

	return mbuf;
}

static int
writemsg(int fd, uchar *mbuf)
{
	int n;
	
	GET(mbuf, n);
	if(write(fd, mbuf, n) != n){
		werrstr("write: %r");
		return -1;
	}

	return 0;
}