/*
 * Copyright (c) 1980,1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)login.c	5.20 (Berkeley) 10/1/87";
#endif not lint

/*
 * login [ name ]
 * login -r hostname	(for rlogind)
 * login -h hostname	(for telnetd, etc.)
 * login -f name	(for pre-authenticated login: datakit, xterm, etc.)
 */

#include <sys/param.h>
#include <sys/quota.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>

#include <sgtty.h>
#include <utmp.h>
#include <signal.h>
#include <pwd.h>
#include <stdio.h>
#include <lastlog.h>
#include <errno.h>
#include <ttyent.h>
#include <syslog.h>
#include <grp.h>

#define TTYGRPNAME	"tty"		/* name of group to own ttys */
#define TTYGID(gid)	tty_gid(gid)	/* gid that owns all ttys */

#define	SCMPN(a, b)	strncmp(a, b, sizeof(a))
#define	SCPYN(a, b)	strncpy(a, b, sizeof(a))

#define NMAX	sizeof(utmp.ut_name)
#define HMAX	sizeof(utmp.ut_host)

#define	FALSE	0
#define	TRUE	-1

char	nolog[] =	"/etc/nologin";
char	qlog[]  =	".hushlogin";
char	maildir[30] =	"/usr/spool/mail/";
char	lastlog[] =	"/usr/adm/lastlog";
struct	passwd nouser = {"", "nope", -1, -1, -1, "", "", "", "" };
struct	sgttyb ttyb;
struct	utmp utmp;
char	minusnam[16] = "-";
char	*envinit[1];			/* now set by setenv calls */
/*
 * This bounds the time given to login.  We initialize it here
 * so it can be patched on machines where it's too small.
 */
int	timeout = 300;

char	term[64];

struct	passwd *pwd;
char	*strcat(), *rindex(), *index();
int	timedout();
char	*ttyname();
char	*crypt();
char	*getpass();
char	*stypeof();
extern	int errno;

struct	tchars tc = {
	CINTR, CQUIT, CSTART, CSTOP, CEOT, CBRK
};
struct	ltchars ltc = {
	CSUSP, CDSUSP, CRPRNT, CFLUSH, CWERASE, CLNEXT
};

struct winsize win = { 0, 0, 0, 0 };

int	rflag;
int	usererr = -1;
char	rusername[NMAX+1], lusername[NMAX+1];
char	rpassword[NMAX+1];
char	name[NMAX+1];
char	me[MAXHOSTNAMELEN];
char	*rhost;

main(argc, argv)
	char *argv[];
{
	extern	char **environ;
	register char *namep;
	int pflag = 0, hflag = 0, fflag = 0, t, f, c;
	int invalid, quietlog;
	FILE *nlfd;
	char *ttyn, *tty;
	int ldisc = 0, zero = 0, i;
	char *p, *domain, *index();

	signal(SIGALRM, timedout);
	alarm(timeout);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	setpriority(PRIO_PROCESS, 0, 0);
	quota(Q_SETUID, 0, 0, 0);
	/*
	 * -p is used by getty to tell login not to destroy the environment
	 * -r is used by rlogind to cause the autologin protocol;
 	 * -f is used to skip a second login authentication 
	 * -h is used by other servers to pass the name of the
	 * remote host to login so that it may be placed in utmp and wtmp
	 */
	(void) gethostname(me, sizeof(me));
	domain = index(me, '.');
	while (argc > 1) {
		if (strcmp(argv[1], "-r") == 0) {
			if (rflag || hflag || fflag) {
				printf("Other options not allowed with -r\n");
				exit(1);
			}
			if (argv[2] == 0)
				exit(1);
			rflag = 1;
			usererr = doremotelogin(argv[2]);
			if ((p = index(argv[2], '.')) && strcmp(p, domain) == 0)
				*p = 0;
			SCPYN(utmp.ut_host, argv[2]);
			argc -= 2;
			argv += 2;
			continue;
		}
		if (strcmp(argv[1], "-h") == 0) {
			if (getuid() == 0) {
				if (rflag || hflag) {
				    printf("Only one of -r and -h allowed\n");
				    exit(1);
				}
				hflag = 1;
				if ((p = index(argv[2], '.')) &&
				    strcmp(p, domain) == 0)
					*p = 0;
				SCPYN(utmp.ut_host, argv[2]);
			}
			argc -= 2;
			argv += 2;
			continue;
		}
		if (strcmp(argv[1], "-f") == 0 && argc > 2) {
			if (rflag) {
				printf("Only one of -r and -f allowed\n");
				exit(1);
			}
			fflag = 1;
			SCPYN(utmp.ut_name, argv[2]);
			argc -= 2;
			argv += 2;
			continue;
		}
		if (strcmp(argv[1], "-p") == 0) {
			argc--;
			argv++;
			pflag = 1;
			continue;
		}
		break;
	}
	ioctl(0, TIOCLSET, &zero);
	ioctl(0, TIOCNXCL, 0);
	ioctl(0, FIONBIO, &zero);
	ioctl(0, FIOASYNC, &zero);
	ioctl(0, TIOCGETP, &ttyb);
	/*
	 * If talking to an rlogin process,
	 * propagate the terminal type and
	 * baud rate across the network.
	 */
	if (rflag)
		doremoteterm(term, &ttyb);
	ttyb.sg_erase = CERASE;
	ttyb.sg_kill = CKILL;
	ioctl(0, TIOCSLTC, &ltc);
	ioctl(0, TIOCSETC, &tc);
	ioctl(0, TIOCSETP, &ttyb);
	for (t = getdtablesize(); t > 2; t--)
		close(t);
	ttyn = ttyname(0);
	if (ttyn == (char *)0 || *ttyn == '\0')
		ttyn = "/dev/tty??";
	tty = rindex(ttyn, '/');
	if (tty == NULL)
		tty = ttyn;
	else
		tty++;
	openlog("login", LOG_ODELAY, LOG_AUTH);
	t = 0;
	invalid = FALSE;
	do {
		ldisc = 0;
		ioctl(0, TIOCSETD, &ldisc);
		if (fflag == 0)
			SCPYN(utmp.ut_name, "");
		/*
		 * Name specified, take it.
		 */
		if (argc > 1) {
			SCPYN(utmp.ut_name, argv[1]);
			argc = 0;
		}
		/*
		 * If remote login take given name,
		 * otherwise prompt user for something.
		 */
		if (rflag && !invalid)
			SCPYN(utmp.ut_name, lusername);
		else {
			getloginname(&utmp);
			if (utmp.ut_name[0] == '-') {
				puts("login names may not start with '-'.");
				invalid = TRUE;
				continue;
			}
		}
		invalid = FALSE;
		if (!strcmp(pwd->pw_shell, "/bin/csh")) {
			ldisc = NTTYDISC;
			ioctl(0, TIOCSETD, &ldisc);
		}
		if (fflag) {
			int uid = getuid();

			if (uid != 0 && uid != pwd->pw_uid)
				fflag = 0;
			/*
			 * Disallow automatic login for root.
			 */
			if (pwd->pw_uid == 0)
				fflag = 0;
		}
		/*
		 * If no remote login authentication and
		 * a password exists for this user, prompt
		 * for one and verify it.
		 */
		if (usererr == -1 && fflag == 0 && *pwd->pw_passwd != '\0') {
			char *pp;

			setpriority(PRIO_PROCESS, 0, -4);
			pp = getpass("Password:");
			namep = crypt(pp, pwd->pw_passwd);
			setpriority(PRIO_PROCESS, 0, 0);
			if (strcmp(namep, pwd->pw_passwd))
				invalid = TRUE;
		}
		/*
		 * If user not super-user, check for logins disabled.
		 */
		if (pwd->pw_uid != 0 && (nlfd = fopen(nolog, "r")) > 0) {
			while ((c = getc(nlfd)) != EOF)
				putchar(c);
			fflush(stdout);
			sleep(5);
			exit(0);
		}
		/*
		 * If valid so far and root is logging in,
		 * see if root logins on this terminal are permitted.
		 */
		if (!invalid && pwd->pw_uid == 0 && !rootterm(tty)) {
			if (utmp.ut_host[0])
				syslog(LOG_CRIT,
				    "ROOT LOGIN REFUSED ON %s FROM %.*s",
				    tty, HMAX, utmp.ut_host);
			else
				syslog(LOG_CRIT,
				    "ROOT LOGIN REFUSED ON %s", tty);
			invalid = TRUE;
		}
		if (invalid) {
			printf("Login incorrect\n");
			if (++t >= 5) {
				if (utmp.ut_host[0])
					syslog(LOG_ERR,
			    "REPEATED LOGIN FAILURES ON %s FROM %.*s, %.*s",
					    tty, HMAX, utmp.ut_host,
					    NMAX, utmp.ut_name);
				else
					syslog(LOG_ERR,
				    "REPEATED LOGIN FAILURES ON %s, %.*s",
						tty, NMAX, utmp.ut_name);
				ioctl(0, TIOCHPCL, (struct sgttyb *) 0);
				close(0), close(1), close(2);
				sleep(10);
				exit(1);
			}
		}
		if (*pwd->pw_shell == '\0')
			pwd->pw_shell = "/bin/sh";
		if (chdir(pwd->pw_dir) < 0 && !invalid ) {
			if (chdir("/") < 0) {
				printf("No directory!\n");
				invalid = TRUE;
			} else {
				printf("No directory! %s\n",
				   "Logging in with home=/");
				pwd->pw_dir = "/";
			}
		}
		/*
		 * Remote login invalid must have been because
		 * of a restriction of some sort, no extra chances.
		 */
		if (!usererr && invalid)
			exit(1);
	} while (invalid);
/* committed to login turn off timeout */
	alarm(0);

	if (quota(Q_SETUID, pwd->pw_uid, 0, 0) < 0 && errno != EINVAL) {
		if (errno == EUSERS)
			printf("%s.\n%s.\n",
			   "Too many users logged on already",
			   "Try again later");
		else if (errno == EPROCLIM)
			printf("You have too many processes running.\n");
		else
			perror("quota (Q_SETUID)");
		sleep(5);
		exit(0);
	}
	time(&utmp.ut_time);
	t = ttyslot();
	if (t > 0 && (f = open("/etc/utmp", O_WRONLY)) >= 0) {
		lseek(f, (long)(t*sizeof(utmp)), 0);
		SCPYN(utmp.ut_line, tty);
		write(f, (char *)&utmp, sizeof(utmp));
		close(f);
	}
	if ((f = open("/usr/adm/wtmp", O_WRONLY|O_APPEND)) >= 0) {
		write(f, (char *)&utmp, sizeof(utmp));
		close(f);
	}
	quietlog = access(qlog, F_OK) == 0;
	if ((f = open(lastlog, O_RDWR)) >= 0) {
		struct lastlog ll;

		lseek(f, (long)pwd->pw_uid * sizeof (struct lastlog), 0);
		if (read(f, (char *) &ll, sizeof ll) == sizeof ll &&
		    ll.ll_time != 0 && !quietlog) {
			printf("Last login: %.*s ",
			    24-5, (char *)ctime(&ll.ll_time));
			if (*ll.ll_host != '\0')
				printf("from %.*s\n",
				    sizeof (ll.ll_host), ll.ll_host);
			else
				printf("on %.*s\n",
				    sizeof (ll.ll_line), ll.ll_line);
		}
		lseek(f, (long)pwd->pw_uid * sizeof (struct lastlog), 0);
		time(&ll.ll_time);
		SCPYN(ll.ll_line, tty);
		SCPYN(ll.ll_host, utmp.ut_host);
		write(f, (char *) &ll, sizeof ll);
		close(f);
	}
	chown(ttyn, pwd->pw_uid, TTYGID(pwd->pw_gid));
	if (!hflag && !rflag)					/* XXX */
		ioctl(0, TIOCSWINSZ, &win);
	chmod(ttyn, 0620);
	setgid(pwd->pw_gid);
	strncpy(name, utmp.ut_name, NMAX);
	name[NMAX] = '\0';
	initgroups(name, pwd->pw_gid);
	quota(Q_DOWARN, pwd->pw_uid, (dev_t)-1, 0);
	setuid(pwd->pw_uid);

	/* destroy environment unless user has asked to preserve it */
	if (!pflag)
		environ = envinit;
	setenv("HOME", pwd->pw_dir, 1);
	setenv("SHELL", pwd->pw_shell, 1);
	if (term[0] == '\0')
		strncpy(term, stypeof(tty), sizeof(term));
	setenv("TERM", term, 0);
	setenv("USER", pwd->pw_name, 1);
	setenv("PATH", ":/usr/ucb:/bin:/usr/bin", 0);

	if ((namep = rindex(pwd->pw_shell, '/')) == NULL)
		namep = pwd->pw_shell;
	else
		namep++;
	strcat(minusnam, namep);
	if (tty[sizeof("tty")-1] == 'd')
		syslog(LOG_INFO, "DIALUP %s, %s", tty, pwd->pw_name);
	if (pwd->pw_uid == 0)
		if (utmp.ut_host[0])
			syslog(LOG_NOTICE, "ROOT LOGIN %s FROM %.*s",
			    tty, HMAX, utmp.ut_host);
		else
			syslog(LOG_NOTICE, "ROOT LOGIN %s", tty);
	if (!quietlog) {
		struct stat st;

		showmotd();
		strcat(maildir, pwd->pw_name);
		if (stat(maildir, &st) == 0 && st.st_size != 0)
			printf("You have %smail.\n",
				(st.st_mtime > st.st_atime) ? "new " : "");
	}
	signal(SIGALRM, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGTSTP, SIG_IGN);
	execlp(pwd->pw_shell, minusnam, 0);
	perror(pwd->pw_shell);
	printf("No shell\n");
	exit(0);
}

getloginname(up)
	register struct utmp *up;
{
	register char *namep;
	char c;

	while (up->ut_name[0] == '\0') {
		namep = up->ut_name;
		printf("login: ");
		while ((c = getchar()) != '\n') {
			if (c == ' ')
				c = '_';
			if (c == EOF)
				exit(0);
			if (namep < up->ut_name+NMAX)
				*namep++ = c;
		}
	}
	strncpy(lusername, up->ut_name, NMAX);
	lusername[NMAX] = 0;
	if ((pwd = getpwnam(lusername)) == NULL)
		pwd = &nouser;
}

timedout()
{

	printf("Login timed out after %d seconds\n", timeout);
	exit(0);
}

int	stopmotd;
catch()
{

	signal(SIGINT, SIG_IGN);
	stopmotd++;
}

rootterm(tty)
	char *tty;
{
	register struct ttyent *t;

	if ((t = getttynam(tty)) != NULL) {
		if (t->ty_status & TTY_SECURE)
			return (1);
	}
	return (0);
}

showmotd()
{
	FILE *mf;
	register c;

	signal(SIGINT, catch);
	if ((mf = fopen("/etc/motd", "r")) != NULL) {
		while ((c = getc(mf)) != EOF && stopmotd == 0)
			putchar(c);
		fclose(mf);
	}
	signal(SIGINT, SIG_IGN);
}

#undef	UNKNOWN
#define UNKNOWN "su"

char *
stypeof(ttyid)
	char *ttyid;
{
	register struct ttyent *t;

	if (ttyid == NULL || (t = getttynam(ttyid)) == NULL)
		return (UNKNOWN);
	return (t->ty_type);
}

doremotelogin(host)
	char *host;
{
	getstr(rusername, sizeof (rusername), "remuser");
	getstr(lusername, sizeof (lusername), "locuser");
	getstr(term, sizeof(term), "Terminal type");
	if (getuid()) {
		pwd = &nouser;
		return(-1);
	}
	pwd = getpwnam(lusername);
	if (pwd == NULL) {
		pwd = &nouser;
		return(-1);
	}
	return(ruserok(host, (pwd->pw_uid == 0), rusername, lusername));
}

getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		if (--cnt < 0) {
			printf("%s too long\r\n", err);
			exit(1);
		}
		*buf++ = c;
	} while (c != 0);
}

char	*speeds[] =
    { "0", "50", "75", "110", "134", "150", "200", "300",
      "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400" };
#define	NSPEEDS	(sizeof (speeds) / sizeof (speeds[0]))

doremoteterm(term, tp)
	char *term;
	struct sgttyb *tp;
{
	register char *cp = index(term, '/'), **cpp;
	char *speed;

	if (cp) {
		*cp++ = '\0';
		speed = cp;
		cp = index(speed, '/');
		if (cp)
			*cp++ = '\0';
		for (cpp = speeds; cpp < &speeds[NSPEEDS]; cpp++)
			if (strcmp(*cpp, speed) == 0) {
				tp->sg_ispeed = tp->sg_ospeed = cpp-speeds;
				break;
			}
	}
	tp->sg_flags = ECHO|CRMOD|ANYP|XTABS;
}

tty_gid(default_gid)
	int default_gid;
{
	struct group *getgrnam(), *gr;
	int gid = default_gid;

	gr = getgrnam(TTYGRPNAME);
	if (gr != (struct group *) 0)
		gid = gr->gr_gid;

	endgrent();

	return (gid);
}
