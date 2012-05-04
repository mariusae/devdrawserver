#include <u.h>
#include <libc.h>
#include <thread.h>

void
usage()
{
	fprint(2, "usage: %s command\n", argv0);
	threadexitsall("usage");
}

void
threadmain(int argc, char **argv)
{
	int p[2], fd[3], pid[2];
	char *a[4], *devdraw;
	
	ARGBEGIN{}ARGEND

	if(argc != 1)
		usage();
		
	// Make sure we don't kill our parent whilst killing
	// our children.
	setsid();

	if(pipe(p) < 0)
		sysfatal("pipe: %r");
	
	fd[0] = dup(p[0], -1);
	fd[1] = dup(p[0], -1);
	fd[2] = dup(2, -1);
	
	a[0] = "rc";
	a[1] = "-c";
	a[2] = argv[0];
	a[3] = nil;

	if((pid[0] = threadspawn(fd, a[0], a)) < 0)
		sysfatal("threadspawn: %r");

	fd[0] = dup(p[1], -1);
	fd[1] = dup(p[1], -1);
	fd[2] = dup(2, -1);

	putenv("NOLIBTHREADDAEMONIZE", "1");
	devdraw = getenv("DEVDRAW");
	if(devdraw == nil)
		devdraw = "devdraw";
	if((pid[1] = threadspawnl(fd, devdraw, devdraw, "(devdraw)", nil)) < 0)
		sysfatal("threadspawnl: %r");

	close(p[0]);
	close(p[1]);

	recvp(threadwaitchan());

	postnote(PNGROUP, pid[0], "hangup");
	postnote(PNGROUP, pid[1], "hangup");
	//recvp(threadwaitchan());

	threadexitsall("");

/*
	if ((pid = fork()) < 0)
		sysfatal("fork: %r");

	switch(pid){
	case 0:
		close(p[0]);
		dup(p[1], 0);
		dup(p[1], 1);

		a[0] = "rc";
		a[1] = "-c";
		a[2] = argv[0];
		a[3] = nil;

		if(exec("rc", a) < 0)
			sysfatal("execl: %r");

	default:
		close(p[1]);
		dup(p[0], 0);
		dup(p[0], 1);

		putenv("NOLIBTHREADDAEMONIZE", "1");
		devdraw = getenv("DEVDRAW");
		if(devdraw == nil)
			devdraw = "devdraw";

		if(execl(devdraw, devdraw, "(devdraw)", nil) < 0)
			sysfatal("execl: %r");
	}

	exits(0);
*/
}
