.\" Copyright (c) 1983 Regents of the University of California.
.\" All rights reserved.  The Berkeley software License Agreement
.\" specifies the terms and conditions for redistribution.
.\"
.\"	@(#)3.t	6.4 (Berkeley) 4/28/86
.\"
.NH 1
Access control
.PP
The printer system maintains protected spooling areas so that
users cannot circumvent printer accounting or
remove files other than their own.
The strategy used to maintain protected
spooling areas is as follows:
.IP \(bu 3
The spooling area is writable only by a \fIdaemon\fP user
and \fIdaemon\fP group.
.IP \(bu 3
The \fIlpr\fP program runs set-user-id to \fIroot\fP and
set-group-id to group \fIdaemon\fP.  The \fIroot\fP access permits
reading any file required. Accessibility is verified
with an \fIaccess\fP\|(2) call.  The group ID
is used in setting up proper ownership of files
in the spooling area for \fIlprm\fP.
.IP \(bu 3
Control files in a spooling area are made with \fIdaemon\fP
ownership and group ownership \fIdaemon\fP.  Their mode is 0660.
This insures control files are not modified by a user
and that no user can remove files except through \fIlprm\fP.
.IP \(bu 3
The spooling programs,
\fIlpd\fP, \fIlpq\fP, and \fIlprm\fP run set-user-id to \fIroot\fP
and set-group-id to group \fIdaemon\fP to access spool files and printers.
.IP \(bu 3
The printer server, \fIlpd\fP,
uses the same verification procedures as \fIrshd\fP\|(8C)
in authenticating remote clients.  The host on which a client
resides must be present in the file /etc/hosts.equiv or /etc/hosts.lpd and
the request message must come from a reserved port number.
.PP
In practice, none of \fIlpd\fP, \fIlpq\fP, or
\fIlprm\fP would have to run as user \fIroot\fP if remote
spooling were not supported.  In previous incarnations of
the printer system \fIlpd\fP ran set-user-id to \fIdaemon\fP,
set-group-id to group \fIspooling\fP, and \fIlpq\fP and \fIlprm\fP ran
set-group-id to group \fIspooling\fP.
