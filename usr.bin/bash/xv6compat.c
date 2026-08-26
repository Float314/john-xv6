/*
 * xv6compat.c - POSIX compatibility layer for the sash shell on
 * xv6-riscv.  Implements everything promised by sash.h on top of the
 * raw xv6 system calls.  This file intentionally does NOT include
 * sash.h; it sees the real kernel/user headers instead.
 */
#include "kernel/types.h"
#include "kernel/stat.h"
/* Rename the kernel's on-disk dirent so we can define our own below. */
#define	dirent	kernel_dirent
#include "kernel/fs.h"
#undef	dirent
#include "kernel/fcntl.h"

#include <stdarg.h>
#include <stddef.h>

/*
 * The real xv6 user headers are NOT included here on purpose: they
 * would drag in conflicting declarations (printf/ffprintf take an fd,
 * malloc takes uint, ...).  Only the raw syscall entry points and the
 * ulib routines we rely on are declared, exactly as usys.S/ulib.o
 * provide them.  The sash_* wrappers defined below (exported to the
 * other modules through sash.h macros) add errno translation on top
 * of these raw calls; this file itself calls the raw entry points.
 */
extern	int	open(const char *, int);
extern	int	close(int);
extern	int	read(int, void *, int);
extern	int	write(int, const void *, int);
extern	int	unlink(const char *);
extern	int	mkdir(const char *);
extern	int	chdir(const char *);
extern	int	link(const char *, const char *);
extern	int	dup(int);
extern	int	mknod(const char *, short, short);
extern	int	sync(void);
extern	int	kill(int);
extern	int	exec(char *, char **);
extern	int	wait(int *);
extern	int	fstat(int, struct stat *);
extern	int	stat(const char *, struct stat *);

extern	char *	sbrk(int);

extern	void *	malloc(unsigned);
extern	void	free(void *);

extern	unsigned	strlen(const char *);
extern	char *		strcpy(char *, const char *);
extern	char *		strcat(char *, const char *);
extern	int		strcmp(const char *, const char *);
extern	int		strncmp(const char *, const char *, size_t);
extern	char *		strchr(const char *, int);
extern	void *		memcpy(void *, const void *, unsigned);
extern	void *		memmove(void *, const void *, unsigned);
extern	void *		memset(void *, int, unsigned);
extern	int		memcmp(const void *, const void *, unsigned);

/*
 * Tiny character predicates (normally supplied by sash.h, which is
 * deliberately not included here).
 */
#define	isDecimal(ch)	(((ch) >= '0') && ((ch) <= '9'))

/*
 * Constants mirrored from sash.h, which is deliberately not included
 * so that its declarations cannot conflict with the real headers.
 */
#define	PATH_LEN	1024

typedef	int	BOOL;

#define	FALSE	((BOOL) 0)
#define	TRUE	((BOOL) 1)

/* Facade types and objects that mirror the declarations in sash.h. */
typedef	int		pid_t;
typedef	long		off_t;
typedef	long		time_t;

typedef	struct _SFILE	FILE;

extern int *		__sash_errno_location(void);
#define	errno		(*__sash_errno_location())

/* Our own stdio/format routines, defined further down. */
extern	void	printf(const char *, ...);
extern	void	fprintf(FILE *, const char *, ...);
extern	int	fileno(FILE *);
extern	char *	strdup(const char *);

extern	FILE *	stdin;
extern	FILE *	stdout;
extern	FILE *	stderr;

#ifndef	EOF
#define	EOF	(-1)
#endif

#define	SEEK_SET	0
#define	SEEK_CUR	1
#define	SEEK_END	2

struct dirent {
	char		d_name[15];	/* NUL terminated, max 14 chars */
};

typedef	struct _SDIR	DIR;

#define	ENOENT		2
#define	EINTR		4
#define	EIO		5
#define	ENOEXEC		8
#define	EBADF		9
#define	EACCES		13
#define	EEXIST		17
#define	EXDEV		18
#define	ENOTDIR		20
#define	EISDIR		21
#define	EINVAL		22
#define	EMFILE		24
#define	EFBIG		27
#define	ENOSPC		28
#define	ESPIPE		29
#define	EROFS		30
#define	ERANGE		34
#define	ENAMETOOLONG	36
#define	ENOSYS		38

/*
 * Errno translation for raw syscalls: the kernel only returns -1,
 * so pick the common failure code for each operation.
 */
int
sash_open(const char * name, int flags)
{
	int	fd = open(name, flags);

	if (fd < 0)
		errno = ENOENT;

	return fd;
}

int
sash_close(int fd)
{
	int	ret = close(fd);

	if (ret < 0)
		errno = EBADF;

	return ret;
}

int
sash_read(int fd, void * buf, int len)
{
	int	ret = read(fd, buf, len);

	if (ret < 0)
		errno = EIO;

	return ret;
}

int
sash_write(int fd, const void * buf, int len)
{
	int	ret = write(fd, buf, len);

	if (ret < 0)
		errno = EIO;

	return ret;
}

int
sash_unlink(const char * name)
{
	int	ret = unlink(name);

	if (ret < 0)
		errno = ENOENT;

	return ret;
}

int
sash_mkdir(const char * name)
{
	int	ret = mkdir(name);

	if (ret < 0)
		errno = EEXIST;

	return ret;
}

int
sash_chdir(const char * name)
{
	int	ret = chdir(name);

	if (ret < 0)
		errno = ENOENT;

	return ret;
}

int
sash_link(const char * oldName, const char * newName)
{
	int	ret = link(oldName, newName);

	if (ret < 0)
		errno = EEXIST;

	return ret;
}

int
sash_dup(int fd)
{
	int	ret = dup(fd);

	if (ret < 0)
		errno = EBADF;

	return ret;
}


#define	S_IFMT		0170000
#define	S_IFIFO		0010000
#define	S_IFCHR		0020000
#define	S_IFBLK		0060000
#define	S_IFDIR		0040000
#define	S_IFREG		0100000

/*
 * --------------------------------------------------------------------
 * errno emulation
 * --------------------------------------------------------------------
 */
static	int	sashErrno;

int *
__sash_errno_location(void)
{
	return &sashErrno;
}

const char *
strerror(int errNum)
{
	static const struct {
		int		code;
		const char *	msg;
	} messages[] = {
		{	0,		"Success"			},
		{	ENOENT,		"No such file or directory"	},
		{	EINTR,		"Interrupted system call"	},
		{	EIO,		"I/O error"			},
		{	ENOEXEC,	"Exec format error"		},
		{	EACCES,		"Permission denied"		},
		{	EEXIST,		"File exists"			},
		{	EXDEV,		"Invalid cross-device link"	},
		{	ENOTDIR,	"Not a directory"		},
		{	EISDIR,		"Is a directory"		},
		{	EINVAL,		"Invalid argument"		},
		{	EMFILE,		"Too many open files"		},
		{	EFBIG,		"File too large"		},
		{	ENOSPC,		"No space left on device"	},
		{	ESPIPE,		"Illegal seek"			},
		{	EROFS,		"Read-only file system"		},
		{	ERANGE,		"Result too large"		},
		{	ENAMETOOLONG,	"File name too long"		},
		{	ENOSYS,		"Function not implemented"	},
	};
	unsigned	i;

	for (i = 0; i < sizeof(messages) / sizeof(messages[0]); i++)
	{
		if (messages[i].code == errNum)
			return messages[i].msg;
	}

	return "Unknown error";
}

void
perror(const char * str)
{
	if ((str != NULL) && (*str != '\0'))
		fprintf(stderr, "%s: %s\n", str, strerror(sashErrno));
	else
		fprintf(stderr, "%s\n", strerror(sashErrno));
}


/*
 * --------------------------------------------------------------------
 * Environment variables (xv6 has none - keep a private table).
 * --------------------------------------------------------------------
 */
#define	ENV_MAX	64

static	char *		envTable[ENV_MAX + 1];
static	int		envCount;
static	int		envInitialised;

char **			environ = envTable;

static const char * const envDefaults[] =
{
	"PATH=/",
	"HOME=/",
	NULL
};

static void
envInit(void)
{
	unsigned i;

	if (envInitialised)
		return;

	for (i = 0; envDefaults[i]; i++)
		envTable[envCount++] = (char *) envDefaults[i];

	envInitialised = 1;
}

char *
getenv(const char * name)
{
	int	len;
	int	i;

	envInit();

	len = strlen(name);

	for (i = 0; i < envCount; i++)
	{
		if ((strncmp(envTable[i], name, len) == 0) &&
			(envTable[i][len] == '='))
		{
			return &envTable[i][len + 1];
		}
	}

	return NULL;
}

int
putenv(char * str)
{
	char *	eq;
	int	len;
	int	i;

	envInit();

	eq = strchr(str, '=');

	if (eq == NULL)
		return -1;

	len = eq - str;

	for (i = 0; i < envCount; i++)
	{
		if ((strncmp(envTable[i], str, len) == 0) &&
			(envTable[i][len] == '='))
		{
			envTable[i] = str;

			return 0;
		}
	}

	if (envCount >= ENV_MAX)
		return -1;

	envTable[envCount++] = str;
	environ = envTable;

	return 0;
}

int
setenv(const char * name, const char * value, int overwrite)
{
	char *	str;

	if (!overwrite && (getenv(name) != NULL))
		return 0;

	str = malloc(strlen(name) + strlen(value) + 2);

	if (str == NULL)
		return -1;

	strcpy(str, name);
	strcat(str, "=");
	strcat(str, value);

	return putenv(str);
}


/*
 * --------------------------------------------------------------------
 * String/memory routines missing from ulib.
 * --------------------------------------------------------------------
 */
char *
strncat(char * dst, const char * src, size_t len)
{
	char *	odst;

	odst = dst;

	while (*dst)
		dst++;

	while (len-- && ((*dst = *src++) != 0))
		dst++;

	*dst = '\0';

	return odst;
}

char *
strncpy(char * dst, const char * src, size_t len)
{
	char *	odst;

	odst = dst;

	while ((len > 0) && *src)
	{
		*dst++ = *src++;
		len--;
	}

	while (len > 0)
	{
		*dst++ = '\0';
		len--;
	}

	return odst;
}

int
strncmp(const char * p1, const char * p2, size_t len)
{
	while ((len > 0) && *p1 && (*p1 == *p2))
	{
		p1++;
		p2++;
		len--;
	}

	if (len == 0)
		return 0;

	return (uchar)*p1 - (uchar)*p2;
}

char *
strrchr(const char * str, int ch)
{
	char *	last;

	last = NULL;

	do {
		if (*str == (char) ch)
			last = (char *) str;
	} while (*str++);

	return last;
}

char *
strstr(const char * str, const char * sub)
{
	const char *	s;
	const char *	t;

	if (*sub == '\0')
		return (char *) str;

	while (*str)
	{
		s = str;
		t = sub;

		while ((*s == *t) && *t)
		{
			s++;
			t++;
		}

		if (*t == '\0')
			return (char *) str;

		str++;
	}

	return NULL;
}

char *
strdup(const char * str)
{
	char *		newStr;
	unsigned	len;

	len = strlen(str) + 1;

	newStr = malloc(len);

	if (newStr)
		memcpy(newStr, str, len);

	return newStr;
}

void *
memchr(const void * str, int ch, size_t len)
{
	const unsigned char *	cp;

	cp = str;

	while (len-- > 0)
	{
		if (*cp == (unsigned char) ch)
			return (void *) cp;

		cp++;
	}

	return NULL;
}


/*
 * --------------------------------------------------------------------
 * printf family with a real format engine (flags, width, precision,
 * length modifiers).  The stock ulib printf only knows a few formats.
 * --------------------------------------------------------------------
 */
struct printCtx {
	char *		buf;
	size_t		pos;
	size_t		cap;
};

static void
emitChar(struct printCtx * ctx, char ch)
{
	if (ctx->pos + 1 < ctx->cap)
		ctx->buf[ctx->pos] = ch;

	ctx->pos++;
}

static void
emitPad(struct printCtx * ctx, int count, char padCh)
{
	while (count-- > 0)
		emitChar(ctx, padCh);
}

static void
emitString(struct printCtx * ctx, const char * str, int len,
	int width, int leftJustify)
{
	int	padLen;

	if (len < 0)
		len = strlen(str);

	padLen = (width > len) ? width - len : 0;

	if (!leftJustify)
		emitPad(ctx, padLen, ' ');

	while (len-- > 0)
		emitChar(ctx, *str++);

	if (leftJustify)
		emitPad(ctx, padLen, ' ');
}

static void
emitNumber(struct printCtx * ctx, unsigned long long val, int base,
	int isSigned, int upper, int width, int prec, int leftJustify,
	int zeroPad)
{
	char			tmp[24];
	const char *		digits;
	int			len;
	int			neg;
	unsigned long long	uval;

	digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
	neg = 0;
	uval = val;

	if (isSigned && ((long long) val < 0))
	{
		neg = 1;
		uval = -(long long) val;
	}

	len = 0;

	do {
		tmp[len++] = digits[uval % base];
		uval /= base;
	} while (uval);

	while (len < prec)
		tmp[len++] = '0';

	if (neg)
		tmp[len++] = '-';

	/*
	 * Zero padding must come after any sign.
	 */
	if (zeroPad && !leftJustify && (width > len))
	{
		if (neg)
		{
			emitChar(ctx, '-');
			neg = 0;
			width--;
		}

		emitPad(ctx, width - len, '0');
		width = 0;
	}

	tmp[len] = '\0';

	emitString(ctx, tmp, len, width, leftJustify);
}

int
vsnprintf(char * buf, size_t cap, const char * fmt, va_list ap)
{
	struct printCtx	ctx;
	const char *	cp;
	char		ch;
	int		leftJustify;
	int		zeroPad;
	int		width;
	int		prec;
	int		isLong;
	long		longVal;
	unsigned long	ulongVal;
	char *		strVal;
	char		charBuf[2];

	ctx.buf = buf;
	ctx.cap = cap;
	ctx.pos = 0;

	if (cap == 0)
		return 0;

	cp = fmt;

	while ((ch = *cp++) != '\0')
	{
		if (ch != '%')
		{
			emitChar(&ctx, ch);

			continue;
		}

		leftJustify = 0;
		zeroPad = 0;
		width = 0;
		prec = -1;
		isLong = 0;

		/* Flags. */
		for (;;)
		{
			ch = *cp++;

			if (ch == '-')
				leftJustify = 1;
			else if (ch == '0')
				zeroPad = 1;
			else if ((ch == '+') || (ch == ' ') || (ch == '#'))
				;
			else
				break;
		}

		/* Width. */
		if (ch == '*')
		{
			width = va_arg(ap, int);

			if (width < 0)
			{
				leftJustify = 1;
				width = -width;
			}

			ch = *cp++;
		}
		else
		{
			while (isDecimal(ch))
			{
				width = width * 10 + (ch - '0');
				ch = *cp++;
			}
		}

		/* Precision. */
		if (ch == '.')
		{
			prec = 0;
			ch = *cp++;

			if (ch == '*')
			{
				prec = va_arg(ap, int);
				ch = *cp++;
			}
			else
			{
				while (isDecimal(ch))
				{
					prec = prec * 10 + (ch - '0');
					ch = *cp++;
				}
			}
		}

		/* Length modifier. */
		if (ch == 'l')
		{
			isLong = 1;
			ch = *cp++;

			if (ch == 'l')
				ch = *cp++;
		}
		else if ((ch == 'z') || (ch == 't'))
		{
			isLong = 1;
			ch = *cp++;
		}

		switch (ch)
		{
			case 'd':
			case 'i':
				if (isLong)
					longVal = va_arg(ap, long);
				else
					longVal = va_arg(ap, int);

				emitNumber(&ctx, (unsigned long long) longVal,
					10, 1, 0, width, prec, leftJustify,
					zeroPad);

				break;

			case 'u':
				if (isLong)
					ulongVal = va_arg(ap, unsigned long);
				else
					ulongVal = va_arg(ap, unsigned int);

				emitNumber(&ctx,
					(unsigned long long) ulongVal,
					10, 0, 0, width, prec, leftJustify,
					zeroPad);

				break;

			case 'o':
				if (isLong)
					ulongVal = va_arg(ap, unsigned long);
				else
					ulongVal = va_arg(ap, unsigned int);

				emitNumber(&ctx,
					(unsigned long long) ulongVal,
					8, 0, 0, width, prec, leftJustify,
					zeroPad);

				break;

			case 'x':
			case 'X':
				if (isLong)
					ulongVal = va_arg(ap, unsigned long);
				else
					ulongVal = va_arg(ap, unsigned int);

				emitNumber(&ctx,
					(unsigned long long) ulongVal,
					16, 0, (ch == 'X'), width, prec,
					leftJustify, zeroPad);

				break;

			case 'c':
				charBuf[0] = (char) va_arg(ap, int);
				charBuf[1] = '\0';

				emitString(&ctx, charBuf, 1, width,
					leftJustify);

				break;

			case 's':
				strVal = va_arg(ap, char *);

				if (strVal == NULL)
					strVal = "(null)";

				if ((prec >= 0) &&
					((long) strlen(strVal) > prec))
				{
					emitString(&ctx, strVal, prec,
						width, leftJustify);
				}
				else
				{
					emitString(&ctx, strVal, -1,
						width, leftJustify);
				}

				break;

			case 'p':
				emitString(&ctx, "0x", 2, 0, 0);
				emitNumber(&ctx,
					(unsigned long long)
					va_arg(ap, void *),
					16, 0, 0, sizeof(void *) * 2, -1,
					0, 0);

				break;

			case '%':
				emitChar(&ctx, '%');

				break;

			case '\0':
				cp--;

				break;

			default:
				/* Unknown conversion - print it verbatim. */
				emitChar(&ctx, '%');
				emitChar(&ctx, ch);

				break;
		}
	}

	if (ctx.pos < ctx.cap)
		buf[ctx.pos] = '\0';
	else
		buf[ctx.cap - 1] = '\0';

	return (int) ctx.pos;
}

int
snprintf(char * buf, size_t len, const char * fmt, ...)
{
	va_list	ap;
	int	ret;

	va_start(ap, fmt);
	ret = vsnprintf(buf, len, fmt, ap);
	va_end(ap);

	return ret;
}

int
sprintf(char * buf, const char * fmt, ...)
{
	va_list	ap;
	int	ret;

	va_start(ap, fmt);
	ret = vsnprintf(buf, 0x7ffffff0, fmt, ap);
	va_end(ap);

	return ret;
}

static void
outputFmt(int fd, const char * fmt, va_list ap)
{
	char	stackBuf[512];
	char *	heapBuf;
	int	needed;

	needed = vsnprintf(stackBuf, sizeof(stackBuf), fmt, ap);

	if (needed < (int) sizeof(stackBuf))
	{
		write(fd, stackBuf, needed);

		return;
	}

	heapBuf = malloc(needed + 1);

	if (heapBuf == NULL)
		return;

	vsnprintf(heapBuf, needed + 1, fmt, ap);
	write(fd, heapBuf, needed);
	free(heapBuf);
}

void
printf(const char * fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	outputFmt(1, fmt, ap);
	va_end(ap);
}

void
fprintf(FILE * fp, const char * fmt, ...)
{
	va_list	ap;
	int	fd;

	fd = fileno(fp);

	va_start(ap, fmt);
	outputFmt(fd, fmt, ap);
	va_end(ap);
}


/*
 * --------------------------------------------------------------------
 * Minimal stdio.  Output is unbuffered (each fputc/fputs goes straight
 * to the write syscall); input is buffered for fgets efficiency.  This
 * avoids any need to flush at exit.
 * --------------------------------------------------------------------
 */
#define	SFILE_BUF	512
#define	SFILE_MAX	12

struct _SFILE {
	int	fd;
	int	inUse;
	int	eofFlag;
	int	errFlag;
	int	bufPos;
	int	bufLen;
	char	buf[SFILE_BUF];
};

static	struct _SFILE	filePool[SFILE_MAX];

static	struct _SFILE	stdFile[3] = {
	{	0,	1,	0,	0,	0,	0,	{ 0 }	},
	{	1,	1,	0,	0,	0,	0,	{ 0 }	},
	{	2,	1,	0,	0,	0,	0,	{ 0 }	}
};

FILE *	stdin = &stdFile[0];
FILE *	stdout = &stdFile[1];
FILE *	stderr = &stdFile[2];

int
fileno(FILE * fp)
{
	if (fp == NULL)
		return -1;

	return fp->fd;
}

static FILE *
fileAlloc(void)
{
	int	i;

	for (i = 0; i < SFILE_MAX; i++)
	{
		if (!filePool[i].inUse)
		{
			memset(&filePool[i], 0, sizeof(filePool[i]));

			filePool[i].inUse = 1;

			return &filePool[i];
		}
	}

	return NULL;
}

FILE *
fopen(const char * name, const char * mode)
{
	FILE *	fp;
	int	flags;
	BOOL	haveMode;
	const char *	cp;

	if ((name == NULL) || (mode == NULL) || (*mode == '\0'))
		return NULL;

	flags = 0;
	haveMode = FALSE;

	for (cp = mode; *cp; cp++)
	{
		switch (*cp)
		{
			case 'r':
				flags = O_RDONLY;
				haveMode = TRUE;

				break;

			case 'w':
				flags = O_WRONLY | O_CREATE | O_TRUNC;
				haveMode = TRUE;

				break;

			case 'a':
				flags = O_WRONLY | O_CREATE;
				haveMode = TRUE;

				break;

			case '+':
				flags = (flags & ~(O_RDONLY | O_WRONLY)) |
					O_RDWR;

				break;

			default:
				break;
		}

		if (haveMode)
			break;
	}

	if (!haveMode)
		return NULL;

	fp = fileAlloc();

	if (fp == NULL)
		return NULL;

	fp->fd = open(name, flags);

	if (fp->fd < 0)
	{
		fp->inUse = 0;

		return NULL;
	}

	return fp;
}

int
fclose(FILE * fp)
{
	int	ret;

	if ((fp == NULL) || !fp->inUse)
		return EOF;

	ret = 0;

	/* The standard streams stay open. */
	if ((fp != stdin) && (fp != stdout) && (fp != stderr))
	{
		if (close(fp->fd) < 0)
			ret = EOF;

		fp->inUse = 0;
	}

	return ret;
}

int
fflush(FILE * fp)
{
	/* Output is unbuffered - nothing to do. */
	if (fp == NULL)
		return 0;

	return 0;
}

int
ferror(FILE * fp)
{
	if (fp == NULL)
		return 0;

	return fp->errFlag;
}

int
feof(FILE * fp)
{
	if (fp == NULL)
		return 0;

	return fp->eofFlag;
}

void
clearerr(FILE * fp)
{
	if (fp == NULL)
		return;

	fp->eofFlag = 0;
	fp->errFlag = 0;
}

static int
fillBuffer(FILE * fp)
{
	int	n;

	n = read(fp->fd, fp->buf, SFILE_BUF);

	if (n <= 0)
	{
		fp->eofFlag = 1;

		if (n < 0)
			fp->errFlag = 1;

		fp->bufPos = 0;
		fp->bufLen = 0;

		return -1;
	}

	fp->bufPos = 0;
	fp->bufLen = n;

	return n;
}

int
fgetc(FILE * fp)
{
	if ((fp == NULL) || !fp->inUse)
		return EOF;

	if (fp->bufPos >= fp->bufLen)
	{
		if (fillBuffer(fp) < 0)
			return EOF;
	}

	return (unsigned char) fp->buf[fp->bufPos++];
}

char *
fgets(char * buf, int len, FILE * fp)
{
	char *	dest;
	int	ch;

	if ((buf == NULL) || (len < 2) || (fp == NULL) || !fp->inUse)
		return NULL;

	dest = buf;

	while (len > 1)
	{
		ch = fgetc(fp);

		if (ch == EOF)
			break;

		*dest++ = ch;
		len--;

		if (ch == '\n')
			break;
	}

	if (dest == buf)
		return NULL;

	*dest = '\0';

	return buf;
}

int
fputc(int ch, FILE * fp)
{
	char	c;

	if ((fp == NULL) || !fp->inUse)
		return EOF;

	c = ch;

	if (write(fp->fd, &c, 1) != 1)
	{
		fp->errFlag = 1;

		return EOF;
	}

	return (unsigned char) c;
}

int
putchar(int ch)
{
	return fputc(ch, stdout);
}

int
fputs(const char * str, FILE * fp)
{
	int	len;

	if ((str == NULL) || (fp == NULL) || !fp->inUse)
		return EOF;

	len = strlen(str);

	if (write(fp->fd, str, len) != len)
	{
		fp->errFlag = 1;

		return EOF;
	}

	return 0;
}


/*
 * --------------------------------------------------------------------
 * qsort: quicksort with an insertion sort for small ranges.
 * --------------------------------------------------------------------
 */
static void
swapMem(char * p1, char * p2, size_t size)
{
	char	tmp;

	while (size--)
	{
		tmp = *p1;
		*p1++ = *p2;
		*p2++ = tmp;
	}
}

static void
sortRange(char * base, size_t n, size_t size,
	int (*compare)(const void *, const void *))
{
	char *	ip;
	char *	jp;

	while (n > 12)
	{
		char *	pivotPtr;
		char *	store;
		char *	cur;
		size_t	nLeft;

		pivotPtr = base + (n - 1) * size;
		store = base;

		/* Lomuto partition; the pivot is excluded from both halves,
		 * which guarantees progress even with many duplicates. */
		for (cur = base; cur < pivotPtr; cur += size)
		{
			if (compare(cur, pivotPtr) < 0)
			{
				swapMem(cur, store, size);
				store += size;
			}
		}

		swapMem(store, pivotPtr, size);

		nLeft = (store - base) / size;

		if (nLeft < n - nLeft - 1)
		{
			sortRange(base, nLeft, size, compare);

			base = store + size;
			n -= nLeft + 1;
		}
		else
		{
			sortRange(store + size, n - nLeft - 1, size,
				compare);

			n = nLeft;
		}
	}

	/* Insertion sort for the small remainder. */
	for (ip = base + size; ip < base + n * size; ip += size)
	{
		for (jp = ip;
			(jp > base) && (compare(jp - size, jp) > 0);
			jp -= size)
		{
			swapMem(jp, jp - size, size);
		}
	}
}

void
qsort(void * basePtr, size_t nmemb, size_t size,
	int (*compare)(const void *, const void *))
{
	if ((basePtr == NULL) || (nmemb < 2) || (size == 0))
		return;

	sortRange(basePtr, nmemb, size, compare);
}


/*
 * --------------------------------------------------------------------
 * File status.  The facade's "struct stat" (see sash.h) has the same
 * layout as this local structure; it just needs a different tag since
 * kernel/stat.h already owns the "struct stat" tag in this file.
 * --------------------------------------------------------------------
 */
struct sash_stat_buf {
	uint		st_dev;
	ushort		st_ino;
	ushort		st_mode;
	short		st_nlink;
	short		st_uid;
	short		st_gid;
	ushort		st_rdev;
	long		st_size;
	long		st_mtime;
};

static void
fillStatBuf(struct sash_stat_buf * sp, const struct stat * kp)
{
	switch (kp->type)
	{
		case T_DIR:
			sp->st_mode = S_IFDIR | 0755;

			break;

		case T_DEVICE:
			sp->st_mode = S_IFCHR | 0660;

			break;

		default:
			sp->st_mode = S_IFREG | 0644;

			break;
	}

	sp->st_dev = kp->dev;
	sp->st_ino = (ushort) kp->ino;
	sp->st_nlink = kp->nlink;
	sp->st_uid = 0;
	sp->st_gid = 0;
	sp->st_rdev = 0;
	sp->st_size = (long) kp->size;
	sp->st_mtime = 0;
}

/*
 * The shell sources call stat(); sash.h routes that name here so that
 * it cannot collide with ulib's stat().
 */
int
sash_stat(const char * name, struct sash_stat_buf * buf)
{
	struct stat	ks;

	if ((name == NULL) || (buf == NULL))
	{
		errno = EINVAL;

		return -1;
	}

	if (stat(name, &ks) < 0)
	{
		errno = ENOENT;

		return -1;
	}

	fillStatBuf(buf, &ks);

	return 0;
}


/*
 * --------------------------------------------------------------------
 * Directory reading over the raw getdents-style reads.
 * --------------------------------------------------------------------
 */
#define	DIRBLKSIZ	512
#define	NDIRPOOL	8

struct _SDIR {
	int	fd;
	int	off;
	int	len;
	char	buf[DIRBLKSIZ];
};

static	struct _SDIR	dirPool[NDIRPOOL];

static	struct dirent	dirEntResult;

static BOOL	dirPoolReady;

static void
dirPoolInit(void)
{
	int	i;

	if (dirPoolReady)
		return;

	for (i = 0; i < NDIRPOOL; i++)
		dirPool[i].fd = -1;

	dirPoolReady = TRUE;
}

DIR *
opendir(const char * name)
{
	DIR *		dp;
	int		i;
	int		fd;
	struct stat	ks;

	if (name == NULL)
		return NULL;

	fd = open(name, O_RDONLY);

	if (fd < 0)
	{
		errno = ENOENT;

		return NULL;
	}

	if ((fstat(fd, &ks) < 0) || (ks.type != T_DIR))
	{
		close(fd);
		errno = ENOTDIR;

		return NULL;
	}

	dirPoolInit();

	dp = NULL;

	for (i = 0; i < NDIRPOOL; i++)
	{
		if (dirPool[i].fd < 0)
		{
			dp = &dirPool[i];

			break;
		}
	}

	if (dp == NULL)
	{
		close(fd);
		errno = EMFILE;

		return NULL;
	}

	dp->fd = fd;
	dp->off = 0;
	dp->len = 0;

	return dp;
}

struct dirent *
readdir(DIR * dirp)
{
	struct kernel_dirent *	kde;

	if ((dirp == NULL) || (dirp->fd < 0))
		return NULL;

	for (;;)
	{
		if (dirp->off >= dirp->len)
		{
			dirp->len = read(dirp->fd, dirp->buf, DIRBLKSIZ);

			if (dirp->len <= 0)
				return NULL;

			dirp->off = 0;
		}

		kde = (struct kernel_dirent *)
			(dirp->buf + dirp->off);

		dirp->off += sizeof(*kde);

		if (kde->inum == 0)
			continue;

		memcpy(dirEntResult.d_name, kde->name, DIRSIZ);
		dirEntResult.d_name[DIRSIZ] = '\0';

		return &dirEntResult;
	}
}

int
closedir(DIR * dirp)
{
	if ((dirp == NULL) || (dirp->fd < 0))
		return -1;

	close(dirp->fd);
	dirp->fd = -1;

	return 0;
}


/*
 * --------------------------------------------------------------------
 * Process related wrappers.
 * --------------------------------------------------------------------
 */
pid_t
waitpid(pid_t pid, int * statusPtr, int options)
{
	int	gotPid;
	int	status;

	for (;;)
	{
		gotPid = wait(&status);

		if (gotPid < 0)
		{
			if (statusPtr != NULL)
				*statusPtr = -1;

			return -1;
		}

		if ((pid <= 0) || (gotPid == pid))
		{
			if (statusPtr != NULL)
				*statusPtr = status;

			return gotPid;
		}

		/* Some other child exited first - keep waiting. */
	}
}

int
sash_kill(int pid, int sig)
{
	(void) sig;

	return kill(pid);
}


/*
 * --------------------------------------------------------------------
 * Misc POSIX bits.
 * --------------------------------------------------------------------
 */
int
access(const char * name, int mode)
{
	int	fd;

	(void) mode;

	if (name == NULL)
		return -1;

	fd = open(name, O_RDONLY);

	if (fd < 0)
	{
		errno = ENOENT;

		return -1;
	}

	close(fd);

	return 0;
}

int
isatty(int fd)
{
	struct stat	ks;

	if (fstat(fd, &ks) < 0)
		return 0;

	return (ks.type == T_DEVICE);
}

char *
getcwd(char * buf, size_t len)
{
	static char	path[PATH_LEN];
	char		comp[DIRSIZ + 1];
	char		dbuf[DIRBLKSIZ];
	struct stat	ksCur;
	struct stat	ksUp;
	struct kernel_dirent *	kde;
	size_t		pathLen;
	size_t		compLen;
	int		fd;
	int		n;
	int		off;
	int		found;
	int		depth;

	if ((buf == NULL) || (len == 0))
	{
		errno = EINVAL;

		return NULL;
	}

	path[0] = '\0';

	for (depth = 0; depth < 32; depth++)
	{
		if (stat(".", &ksCur) < 0)
			return NULL;

		fd = open("..", O_RDONLY);

		if (fd < 0)
		{
			errno = EACCES;

			return NULL;
		}

		if (fstat(fd, &ksUp) < 0)
		{
			close(fd);
			errno = EACCES;

			return NULL;
		}

		/* When ".." is us, we are at the root. */
		if (ksUp.ino == ksCur.ino)
		{
			close(fd);

			break;
		}

		found = FALSE;

		while (!found &&
			((n = read(fd, dbuf, DIRBLKSIZ)) > 0))
		{
			for (off = 0;
				off + sizeof(struct kernel_dirent) <= n;
				off += sizeof(struct kernel_dirent))
			{
				kde = (struct kernel_dirent *)
					(dbuf + off);

				if (kde->inum == ksCur.ino)
				{
					memcpy(comp, kde->name, DIRSIZ);
					comp[DIRSIZ] = '\0';
					found = TRUE;

					break;
				}
			}
		}

		close(fd);

		if (!found)
		{
			errno = ENOENT;

			return NULL;
		}

		compLen = strlen(comp);
		pathLen = strlen(path);

		if (pathLen + compLen + 2 > sizeof(path))
		{
			errno = ENAMETOOLONG;

			return NULL;
		}

		memmove(path + compLen + 1, path, pathLen + 1);
		path[0] = '/';
		memcpy(path + 1, comp, compLen);

		if (chdir("..") < 0)
		{
			errno = EACCES;

			return NULL;
		}
	}

	/* Restore the original working directory. */
	chdir((path[0] != '\0') ? path : "/");

	if (strlen(path) + 1 > len)
	{
		errno = ERANGE;

		return NULL;
	}

	strcpy(buf, (path[0] != '\0') ? path : "/");

	return buf;
}

int
rename(const char * oldName, const char * newName)
{
	if (link(oldName, newName) < 0)
	{
		errno = EEXIST;

		return -1;
	}

	unlink(oldName);

	return 0;
}

off_t
lseek(int fd, off_t offset, int whence)
{
	static char	zeros[512];
	struct stat	ks;
	off_t		size;
	int		chunk;

	if (whence != SEEK_SET)
	{
		errno = ESPIPE;

		return -1;
	}

	if ((fstat(fd, &ks) < 0) || (offset < 0))
	{
		errno = EINVAL;

		return -1;
	}

	size = (off_t) ks.size;

	/*
	 * Repositioning inside existing data would require reading
	 * through the whole file, which the callers never rely on.
	 * Extending a freshly created output file with zero bytes,
	 * however, is exactly what "dd seek=" needs.
	 */
	if (offset <= size)
	{
		errno = ESPIPE;

		return -1;
	}

	memset(zeros, 0, sizeof(zeros));

	while (size < offset)
	{
		chunk = (offset - size > (off_t) sizeof(zeros)) ?
			sizeof(zeros) : (int) (offset - size);

		if (write(fd, zeros, chunk) != chunk)
		{
			errno = ENOSPC;

			return -1;
		}

		size += chunk;
	}

	return offset;
}

int
execvp(const char * file, char * const * argv)
{
	const char *	pathList;
	const char *	p;
	const char *	seg;
	char		buf[PATH_LEN];
	int		segLen;
	int		buildLen;
	int		fileLen;

	if (file == NULL)
	{
		errno = EINVAL;

		return -1;
	}

	fileLen = strlen(file);

	/* Any slash means an explicit path. */
	if (strchr(file, '/') != NULL)
	{
		exec((char *) file, (char **) argv);
		errno = ENOENT;

		return -1;
	}

	pathList = getenv("PATH");

	if ((pathList == NULL) || (*pathList == '\0'))
		pathList = "/";

	p = pathList;

	for (;;)
	{
		seg = p;

		while (*p && (*p != ':'))
			p++;

		segLen = p - seg;
		buildLen = 0;

		if (segLen > 0)
		{
			memcpy(buf, seg, segLen);
			buildLen = segLen;
		}
		else
		{
			/* Empty PATH element means the current directory. */
			buf[buildLen++] = '.';
		}

		if (buf[buildLen - 1] != '/')
			buf[buildLen++] = '/';

		if (buildLen + fileLen + 1 <= (int) sizeof(buf))
		{
			memcpy(buf + buildLen, file, fileLen + 1);

			exec(buf, (char **) argv);
		}

		if (*p == '\0')
			break;

		p++;
	}

	errno = ENOENT;

	return -1;
}


/*
 * --------------------------------------------------------------------
 * Time.  xv6 has no real-time clock: report the epoch.
 * --------------------------------------------------------------------
 */
time_t
time(time_t * tloc)
{
	if (tloc != NULL)
		*tloc = 0;

	return 0;
}

char *
ctime(const time_t * clock)
{
	static char	timeBuf[] = "Thu Jan  1 00:00:00 1970\n";

	(void) clock;

	return timeBuf;
}
