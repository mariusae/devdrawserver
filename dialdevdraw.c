#include <u.h>
#include <libc.h>

void
usage()
{
	fprint(2, "usage: %s command\n", argv0);
	exits("usage");
}

void
main(int argc, char **argv)
{
	int p[2], pid;
	char *a[4], *devdraw;
	
	ARGBEGIN{}ARGEND

	if(argc != 1)
		usage();

	if(pipe(p) < 0)
		sysfatal("pipe: %r");

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
}