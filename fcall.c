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



Channel *
procproxyfcalls(int readfd, int writefd)
{
	void **argv;
	Channel *c;

	if ((argv = malloc(3*sizeof(void*))) == nil)
		sysfatal("malloc: %r");

	c = chancreate(sizeof(void*), 0);
	argv[0] = (void*)(intptr)readfd;
	argv[1] = (void*)(intptr)writefd;
	argv[2] = c;
	proccreate(proxyfcallsproc, argv, 1<<15);

	return c;
}

void
proxyfcalls(int readfd, int writefd)
{
	int r;
	uchar *mbuf = nil;

	for(;;){
		if((mbuf = readmsg(readfd)) == nil)
			break;
		r = writemsg(writefd, mbuf);
		free(mbuf);
		if(r < 0)
			break;
	}

}

static void
proxyfcallsproc(void *arg)
{
	int writefd, readfd;
	Channel *c;
	void **argv = arg;

	readfd = (intptr)argv[0];
	writefd = (intptr)argv[1];
	c = argv[2];
	free(argv);

	proxyfcalls(readfd, writefd);
	sendp(c, smprint("%r"));
}

static void
readproc(void *arg)
{
	uchar *mbuf;
	int fd;
	Channel *ch;
	void **argv = (void **)arg;
	
	ch = (Channel *)argv[0];
	fd = (uint)(intptr)argv[1];

	while((mbuf = readmsg(fd)) != nil)
		sendp(ch, mbuf);

	sendp(ch, nil);
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