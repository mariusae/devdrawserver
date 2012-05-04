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

void serve();
void rmsock();
static char sockpath[200];

void
usage()
{
	fprint(2, "usage: %s\n", argv0);
	threadexitsall("usage");
}

void
threadmain(int argc, char **argv)
{
	fmtinstall('W', drawfcallfmt);
	fmtinstall('H', encodefmt);
	
//	snprint(sockpath, sizeof sockpath, "%s/devdrawserver", getns());
	strcpy(sockpath, "/tmp/devdrawserver");

	serve();
	threadexitsall("");
}

void
rmsock()
{
	if(remove(sockpath) < 0)
		fprint(2, "remove: %r\n");
}

void
serve()
{
	int afd, lfd;
	char dir[100], adir[100], buf[200];
	Alt a[3];
	
	snprint(buf, sizeof buf, "unix!%s", sockpath);

	atexit(rmsock);
	if((afd = announce(buf, adir)) < 0)
		sysfatal("announce: %r");

	if ((lfd = listen(adir, dir)) < 0)
		goto Error;

	a[0].c = procproxyfcalls(lfd, 1);
	a[0].op = CHANRCV;
	a[1].c = procproxyfcalls(0, lfd);
	a[1].op = CHANRCV;
	a[2].op = CHANEND;

	werrstr(a[alt(a)].v);
	close(lfd);

  Error:
	close(afd);
	threadexitsall("%r");
}

void
_flushmemscreen(Rectangle r)
{}

