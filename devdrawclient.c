#include <u.h>
#include <libc.h>

void
main(int argc, char **argv)
{
	int dfd;
	char *devdraw;

	if ((dfd = dial("unix!/tmp/devdrawserver", nil, nil, nil)) < 0)
		sysfatal("dial: %r");

	dup(dfd, 0);
	dup(dfd, 1);

	putenv("NOLIBTHREADDAEMONIZE", "1");
	devdraw = getenv("DEVDRAW");
	if(devdraw == nil)
		devdraw = "devdraw";
	fprint(2, "exec %s\n", devdraw);
	if(execl(devdraw, devdraw, "(devdraw)", nil) < 0)
		sysfatal("execl: %r");

	exits(0);
}
