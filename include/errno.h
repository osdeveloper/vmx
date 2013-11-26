/******************************************************************************
 *   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 *   This file is part of Real VMX.
 *   Copyright (C) 2013 Surplus Users Ham Society
 *
 *   Real VMX is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Real VMX is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _errno_h
#define _errno_h

#define	EPERM		1		/* Not owner */
#define	ENOENT		2		/* No such file or directory */
#define	ESRCH		3		/* No such process */
#define	EINTR		4		/* Interrupted system call */
#define	EIO		5		/* I/O error */
#define	ENXIO		6		/* No such device or address */
#define	E2BIG		7		/* Arg list too long */
#define	ENOEXEC		8		/* Exec format error */
#define	EBADF		9		/* Bad file number */
#define	ECHILD		10		/* No children */
#define	EAGAIN		11		/* No more processes */
#define	ENOMEM		12		/* Not enough core */
#define	EACCES		13		/* Permission denied */
#define	EFAULT		14		/* Bad address */
#define	ENOTEMPTY	15		/* Directory not empty */
#define	EBUSY		16		/* Mount device busy */
#define	EEXIST		17		/* File exists */
#define	EXDEV		18		/* Cross-device link */
#define	ENODEV		19		/* No such device */
#define	ENOTDIR		20		/* Not a directory*/
#define	EISDIR		21		/* Is a directory */
#define	EINVAL		22		/* Invalid argument */
#define	ENFILE		23		/* File table overflow */
#define	EMFILE		24		/* Too many open files */
#define	ENOTTY		25		/* Not a typewriter */
#define	ENAMETOOLONG	26		/* File name too long */
#define	EFBIG		27		/* File too large */
#define	ENOSPC		28		/* No space left on device */
#define	ESPIPE		29		/* Illegal seek */
#define	EROFS		30		/* Read-only file system */
#define	EMLINK		31		/* Too many links */
#define	EPIPE		32		/* Broken pipe */
#define	EDEADLK		33		/* Resource deadlock avoided */
#define	ENOLCK		34		/* No locks available */
#define	ENOTSUP		35		/* Unsupported value */
#define	EMSGSIZE	36		/* Message size */
#define	EDOM		37		/* Argument too large */
#define	ERANGE		38		/* Result too large */
#define	EDESTADDRREQ	40		/* Destination address required */
#define	EPROTOTYPE	41		/* Protocol wrong type for socket */
#define	ENOPROTOOPT	42		/* Protocol not available */
#define	EPROTONOSUPPORT	43		/* Protocol not supported */
#define	ESOCKTNOSUPPORT	44		/* Socket type not supported */
#define	EOPNOTSUPP	45		/* Operation not supported on socket */
#define	EPFNOSUPPORT	46		/* Protocol family not supported */
#define	EAFNOSUPPORT	47		/* Addr family not supported */
#define	EADDRINUSE	48		/* Address already in use */
#define	EADDRNOTAVAIL	49		/* Can't assign requested address */
#define	ENOTSOCK	50		/* Socket operation on non-socket */
#define	ENETUNREACH	51		/* Network is unreachable */
#define	ENETRESET	52		/* Network dropped connection on reset*/
#define	ECONNABORTED	53		/* Software caused connection abort */
#define	ECONNRESET	54		/* Connection reset by peer */
#define	ENOBUFS		55		/* No buffer space available */
#define	EISCONN		56		/* Socket is already connected */
#define	ENOTCONN	57		/* Socket is not connected */
#define	ESHUTDOWN	58		/* Can't send after socket shutdown */
#define	ETOOMANYREFS	59		/* Too many references: can't splice */
#define	ETIMEDOUT	60		/* Connection timed out */
#define	ECONNREFUSED	61		/* Connection refused */
#define	ENETDOWN	62		/* Network is down */
#define	ETXTBSY		63		/* Text file busy */
#define	ELOOP		64		/* Too many levels of symbolic links */
#define	EHOSTUNREACH	65		/* No route to host */
#define	ENOTBLK		66		/* Block device required */
#define	EHOSTDOWN	67		/* Host is down */
#define	EINPROGRESS	68		/* Operation now in progress */
#define	EALREADY	69		/* Operation already in progress */
#define	EWOULDBLOCK	70		/* Operation would block */
#define	ENOSYS		71		/* Function not implemented */
#define	ECANCELED	72		/* Operation canceled */
#define ENOSR           74              /* Insufficient memory */
#define ENOSTR          75              /* STREAMS device required */
#define EPROTO          76              /* Generic STREAMS error */
#define EBADMSG         77              /* Invalid STREAMS message */
#define ENODATA         78              /* Missing expected message data */
#define ETIME           79              /* STREAMS timeout occurred */
#define ENOMSG          80              /* Unexpected message type */ 
#define EFPOS		81		/* File positioning error */
#define EILSEQ		82		/* Illegal byte sequence. */
#define EDQUOT          83              /* Routing error */

#define	ERRMAX		84

#endif /* _errno_h */

