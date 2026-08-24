/*
 * Copyright (c) 1999 by David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Definitions for the stand-alone shell, ported to xv6-riscv.
 *
 * On xv6 there is no libc: this header is a self-contained POSIX-like
 * facade implemented on top of the raw xv6 system calls by
 * xv6compat.c.  Nothing from user/user.h is included here so that the
 * namespace stays fully under our control (user programs that need
 * ulib/printf keep their own headers; sash overrides them instead).
 */

#ifndef	SASH_H
#define	SASH_H

#include <stddef.h>
#include <stdarg.h>

/*
 * Feature switches: the Linux-only extras of upstream sash
 * (gzip via zlib, ext2 attributes) are disabled under xv6.
 */
#undef	HAVE_GZIP
#undef	HAVE_EXT2

#define	PATH_LEN	1024
#define	CMD_LEN		10240
#define	ALIAS_ALLOC	20
#define	EXPAND_ALLOC	1024
#define	STDIN		0
#define	STDOUT		1
#define	MAX_SOURCE	10
#define	BUF_SIZE	8192


#define	isBlank(ch)	(((ch) == ' ') || ((ch) == '\t'))
#define	isDecimal(ch)	(((ch) >= '0') && ((ch) <= '9'))
#define	isOctal(ch)	(((ch) >= '0') && ((ch) <= '7'))
#define	isWildCard(ch)	(((ch) == '*') || ((ch) == '?') || ((ch) == '['))

/* Minimal ctype support used by the command implementations. */
#define	isupper(ch)	(((ch) >= 'A') && ((ch) <= 'Z'))
#define	islower(ch)	(((ch) >= 'a') && ((ch) <= 'z'))
#define	tolower(ch)	((ch) - 'A' + 'a')
#define	toupper(ch)	((ch) - 'a' + 'A')
#define	isspace(ch)	(((ch) == ' ') || ((ch) == '\t') || \
			((ch) == '\n') || ((ch) == '\r') || \
			((ch) == '\f') || ((ch) == '\v'))
#define	isprint(ch)	((((unsigned char)(ch)) >= 0x20) && \
			(((unsigned char)(ch)) < 0x7f))
#define	isalpha(ch)	(isupper(ch) || islower(ch))

#ifndef MAX
#define MAX(x, y)	((x) > (y) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x, y)	((x) < (y) ? (x) : (y))
#endif

typedef	int	BOOL;

#define	FALSE	((BOOL) 0)
#define	TRUE	((BOOL) 1)


/*
 * Types which xv6 does not define but the code refers to.
 */
typedef	int		pid_t;
typedef	unsigned short	ino_t;
typedef	unsigned int	dev_t;
typedef	unsigned short	mode_t;
typedef	long		time_t;
typedef	long		off_t;


/*
 * errno emulation (single-threaded shell, one static value).
 */
extern int *		__sash_errno_location(void);
#define	errno		(*__sash_errno_location())

#define	ENOENT		2
#define	EINTR		4
#define	EIO		5
#define	EACCES		13
#define	EEXIST		17
#define	ENOTDIR		20
#define	EISDIR		21
#define	EINVAL		22
#define	EMFILE		24
#define	EFBIG		27
#define	ENOSPC		28
#define	ESPIPE		29
#define	EROFS		30
#define	ENOEXEC		8
#define	EXDEV		18
#define	ERANGE		34
#define	ENAMETOOLONG	36
#define	ENOSYS		38

extern const char *	strerror(int errNum);
extern void		perror(const char * str);


/*
 * A minimal stdio implementation over the xv6 read/write calls,
 * provided by xv6compat.c.
 */
typedef	struct _SFILE	FILE;

#define	EOF		(-1)

extern FILE *		stdin;
extern FILE *		stdout;
extern FILE *		stderr;

extern FILE *		fopen(const char * name, const char * mode);
extern int		fclose(FILE * fp);
extern int		fileno(FILE * fp);
extern char *		fgets(char * buf, int len, FILE * fp);
extern int		fgetc(FILE * fp);
extern int		fputc(int ch, FILE * fp);
extern int		fputs(const char * str, FILE * fp);
extern int		putchar(int ch);
extern int		fflush(FILE * fp);
extern int		feof(FILE * fp);
extern int		ferror(FILE * fp);
extern void		clearerr(FILE * fp);

extern void		printf(const char *, ...) 
				__attribute__((format(printf, 1, 2)));
extern void		fprintf(FILE *, const char *, ...)
				__attribute__((format(printf, 2, 3)));
extern int		sprintf(char * buf, const char * fmt, ...)
				__attribute__((format(printf, 2, 3)));
extern int		snprintf(char * buf, size_t len,
				const char * fmt, ...)
				__attribute__((format(printf, 3, 4)));
extern int		vsnprintf(char * buf, size_t len,
				const char * fmt, va_list ap);


/*
 * stdlib pieces the shell needs.
 */
extern void *		malloc(size_t size);
extern void		free(void * ptr);
extern void *		realloc(void * ptr, size_t size);
extern void		qsort(void * base, size_t nmemb, size_t size,
				int (*cmp)(const void *, const void *));
extern void		exit(int status) __attribute__((noreturn));
extern char *		getenv(const char * name);
extern int		putenv(char * str);
extern int		setenv(const char * name, const char * value,
				int overwrite);
extern char **		environ;


/*
 * string/memory routines (some are also in ulib; identical semantics).
 */
extern char *		strcpy(char * dst, const char * src);
extern char *		strncpy(char * dst, const char * src, size_t len);
extern char *		strcat(char * dst, const char * src);
extern char *		strncat(char * dst, const char * src, size_t len);
extern int		strcmp(const char * p1, const char * p2);
extern int		strncmp(const char * p1, const char * p2, size_t len);
extern unsigned	strlen(const char * str);
extern char *		strchr(const char * str, int ch);
extern char *		strrchr(const char * str, int ch);
extern char *		strstr(const char * str, const char * sub);
extern char *		strdup(const char * str);
extern void *		memset(void * dst, int val, size_t len);
extern void *		memcpy(void * dst, const void * src, size_t len);
extern void *		memmove(void * dst, const void * src, size_t len);
extern int		memcmp(const void * p1, const void * p2, size_t len);
extern void *		memchr(const void * str, int ch, size_t len);
extern int		atoi(const char * str);


/*
 * File status.  xv6 only reports type/nlink/ino/dev/size, so
 * xv6compat.c synthesizes permission bits and a POSIX-style mode word.
 */
struct stat {
	dev_t		st_dev;
	ino_t		st_ino;
	unsigned short	st_mode;
	short		st_nlink;
	short		st_uid;
	short		st_gid;
	unsigned short	st_rdev;
	long		st_size;
	time_t		st_mtime;
};

#define	S_IFMT		0170000
#define	S_IFREG		0100000
#define	S_IFDIR		0040000
#define	S_IFBLK		0060000
#define	S_IFCHR		0020000
#define	S_IFIFO		0010000

#define	S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define	S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define	S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define	S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define	S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
/* Symbolic links and sockets do not exist under xv6: S_ISLNK/S_ISSOCK
 * are intentionally NOT defined so guarded code paths compile out. */

#define	S_IRWXU		00700
#define	S_IRUSR		00400
#define	S_IWUSR		00200
#define	S_IXUSR		00100
#define	S_IRWXG		00070
#define	S_IRGRP		00040
#define	S_IWGRP		00020
#define	S_IXGRP		00010
#define	S_IRWXO		00007
#define	S_IROTH		00004
#define	S_IWOTH		00002
#define	S_IXOTH		00001
#define	S_ISUID		04000
#define	S_ISGID		02000
#define	S_ISVTX		01000
#define	S_IEXEC		S_IXUSR

extern	int	sash_stat(const char * name, struct stat * buf);
/*
 * Route plain stat() to our wrapper so that the facade layout is used
 * and no link-time collision with ulib's stat() can occur.
 */
#define	stat(name, buf)		sash_stat((name), (buf))
/* No symlink distinction under xv6. */
#define	lstat(name, buf)	sash_stat((name), (buf))


/*
 * Directory reading.
 */
struct dirent {
	char		d_name[15];	/* NUL terminated, max 14 chars */
};

typedef	struct _SDIR	DIR;

extern DIR *		opendir(const char * name);
extern struct dirent *	readdir(DIR * dirp);
extern int		closedir(DIR * dirp);


/*
 * Raw file operations (thin wrappers around the xv6 syscalls).
 */
#define	O_RDONLY	0x000
#define	O_WRONLY	0x001
#define	O_RDWR		0x002
#define	O_CREATE	0x200
#define	O_TRUNC		0x400
/* xv6 has no exclusive-create flag; ignore it. */
#define	O_EXCL		0

/* POSIX spelling of the kernel's O_CREATE. */
#ifndef	O_CREAT
#define	O_CREAT		O_CREATE
#endif

/*
 * Raw file operations.  The kernel reports failures only as -1 with
 * no error code, so thin wrappers in xv6compat.c translate common
 * cases into reasonable errno values for perror().
 */
extern	int	sash_open(const char * name, int flags);
#define	open(name, flags)		sash_open((name), (flags))
extern	int	sash_close(int fd);
#define	close(fd)			sash_close((fd))
extern	int	sash_read(int fd, void * buf, int len);
#define	read(fd, buf, len)		sash_read((fd), (buf), (len))
extern	int	sash_write(int fd, const void * buf, int len);
#define	write(fd, buf, len)		sash_write((fd), (buf), (len))
extern	int	sash_unlink(const char * name);
#define	unlink(name)			sash_unlink((name))
extern	int	sash_mkdir(const char * name);
#define	mkdir(name, mode)		sash_mkdir((name))
#define	rmdir(name)			sash_unlink(name)
extern	int	sash_chdir(const char * name);
#define	chdir(name)			sash_chdir((name))
extern	int	sash_link(const char * oldName, const char * newName);
#define	link(oldName, newName)		sash_link((oldName), (newName))
extern	int	sash_dup(int fd);
#define	dup(fd)				sash_dup((fd))

extern	int	mknod(const char * name, short major, short minor);
extern	int	sync(void);
extern	int	getpid(void);

#define	creat(name, mode) \
	sash_open((name), O_CREATE | O_WRONLY | O_TRUNC)


/*
 * Process related wrappers.
 */
extern	pid_t	fork(void);
extern	pid_t	waitpid(pid_t pid, int * status, int options);
/* xv6 reports killed processes with an exit status of -1. */
#define	WIFSIGNALED(status)	((status) == -1)
#define	WIFEXITED(status)	((status) != -1)
#define	WTERMSIG(status)	9
#define	WEXITSTATUS(status)	(status)

extern	int	execvp(const char * name, char * const * argv);
extern	int	sash_kill(int pid, int sig);
#define	kill(pid, sig)	sash_kill((pid), (sig))

/* Signal numbers are accepted but ignored (no signal delivery in xv6). */
#define	SIGHUP	1
#define	SIGINT	2
#define	SIGQUIT	3
#define	SIGKILL	9
#define	SIGSTOP	17
#define	SIGCONT	18
#define	SIGTERM	15
#define	SIGUSR1	10
#define	SIGUSR2	12

extern	char *	getcwd(char * buf, size_t len);
extern	int	isatty(int fd);
extern	int	access(const char * name, int mode);
#define	F_OK	0
#define	X_OK	1
#define	W_OK	2
#define	R_OK	4

extern	int	rename(const char * oldName, const char * newName);
extern	off_t	lseek(int fd, off_t offset, int whence);
#define	SEEK_SET	0
#define	SEEK_CUR	1
#define	SEEK_END	2


/*
 * Time: xv6 has no real-time clock, time() always returns 0 and
 * ctime() returns a fixed epoch string with the standard layout.
 */
extern	time_t	time(time_t * tloc);
extern	char *	ctime(const time_t * clock);


/*
 * Built-in command functions.
 */
extern	void	do_alias(int argc, const char ** argv);
extern	void	do_aliasall(int argc, const char ** argv);
extern	void	do_cd(int argc, const char ** argv);
extern	void	do_exec(int argc, const char ** argv);
extern	void	do_exit(int argc, const char ** argv);
extern	void	do_prompt(int argc, const char ** argv);
extern	void	do_source(int argc, const char ** argv);
extern	void	do_umask(int argc, const char ** argv);
extern	void	do_unalias(int argc, const char ** argv);
extern	void	do_help(int argc, const char ** argv);
extern	void	do_ln(int argc, const char ** argv);
extern	void	do_cp(int argc, const char ** argv);
extern	void	do_mv(int argc, const char ** argv);
extern	void	do_rm(int argc, const char ** argv);
extern	void	do_mkdir(int argc, const char ** argv);
extern	void	do_rmdir(int argc, const char ** argv);
extern	void	do_mknod(int argc, const char ** argv);
extern	void	do_sum(int argc, const char ** argv);
extern	void	do_sync(int argc, const char ** argv);
extern	void	do_printenv(int argc, const char ** argv);
extern	void	do_more(int argc, const char ** argv);
extern	void	do_cmp(int argc, const char ** argv);
extern	void	do_touch(int argc, const char ** argv);
extern	void	do_ls(int argc, const char ** argv);
extern	void	do_dd(int argc, const char ** argv);
extern	void	do_tar(int argc, const char ** argv);
extern	void	do_setenv(int argc, const char ** argv);
extern	void	do_pwd(int argc, const char ** argv);
extern	void	do_echo(int argc, const char ** argv);
extern	void	do_kill(int argc, const char ** argv);
extern	void	do_grep(int argc, const char ** argv);
extern	void	do_file(int argc, const char ** argv);
extern	void	do_find(int argc, const char ** argv);
extern	void	do_ed(int argc, const char ** argv);
extern	void	do_where(int argc, const char ** argv);


/*
 * Global utility routines.
 */
extern	const char *	modeString(int mode);
extern	const char *	timeString(time_t timeVal);
extern	BOOL		isDirectory(const char * name);
extern	BOOL		isDevice(const char * name);
extern	int		nameSort(const void * p1, const void * p2);
extern	char *		getChunk(int size);
extern	char *		chunkstrdup(const char *);
extern	void		freeChunks(void);
extern	int		fullWrite(int fd, const char * buf, int len);
extern	int		fullRead(int fd, char * buf, int len);
extern	BOOL		match(const char * text, const char * pattern);

extern	const char *	buildName
	(const char * dirName, const char * fileName);

extern	BOOL	makeArgs
	(const char * cmd, int * argcPtr, const char *** argvPtr);

extern	BOOL	copyFile
	(const char * srcName, const char * destName, BOOL setModes);

extern	BOOL	makeString
	(int argc, const char ** argv, char * buf, int bufLen);

extern	int	expandWildCards
	(const char * fileNamePattern, const char *** retFileTable);


/*
 * Global variable to indicate that an interrupt occurred.
 * Under xv6 there is no signal delivery, so this simply stays FALSE.
 */
extern	BOOL	intFlag;

#endif

/* END CODE */
