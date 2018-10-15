/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2018  Ondřej Kuzník <okuznik@symas.com>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
*/

/** @file
 *
 * This "filesystem" provides only a single file. The mountpoint
 * needs to be a file rather than a directory. Its storage is backed
 * by a circular buffer of a predetermined size n and is append-only.
 * Reads will read from that buffer, effectively reading the last n
 * bytes written to it.
 *
 * Compile with:
 *
 *     gcc -Wall buffer.c `pkg-config fuse3 --cflags --libs` -o buffer
 *
 * ## Source code ##
 * \include buffer.c
 */


#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <fuse_lowlevel.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

off_t buffer_offset = 0;
off_t buffer_size = 100;
int wrapped = 0;

char *buffer;

static int buffer_getattr(const char *path, struct stat *stbuf,
			struct fuse_file_info *fi)
{
	(void) fi;

	if(strcmp(path, "/") != 0)
		return -ENOENT;

	stbuf->st_mode = S_IFREG | 0644;
	stbuf->st_nlink = 1;
	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
	stbuf->st_size = buffer_size;
	stbuf->st_blocks = 0;
	stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);

	return 0;
}

static int buffer_truncate(const char *path, off_t size,
			 struct fuse_file_info *fi)
{
	(void) size;
	(void) fi;

	if(strcmp(path, "/") != 0)
		return -ENOENT;

	if ( size >= buffer_size ) {
		return 0;
	}

	if ( wrapped || size < buffer_offset ) {
		wrapped = 0;
		buffer_offset = size;
	}

	return 0;
}

static int buffer_open(const char *path, struct fuse_file_info *fi)
{
	(void) fi;

	if(strcmp(path, "/") != 0)
		return -ENOENT;

	return 0;
}

static int buffer_read(const char *path, char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	(void) fi;

	printf("receiving read on '%s' offset=%zu size=%zu wrapped=%d\n", path, offset, size, wrapped);
	if(strcmp(path, "/") != 0)
		return -ENOENT;

	if (offset >= buffer_size)
		return 0;

	if (!wrapped) {
		if (offset >= buffer_offset) {
			return 0;
		}
		if (size > buffer_offset - offset) {
			size = buffer_offset - offset;
		}
		memcpy(buf, buffer + offset, size);
		return size;
	}

	if (offset + size > buffer_size)
		size = buffer_size - offset;

	if (buffer_offset + size <= buffer_size) {
		memcpy(buf, buffer+buffer_offset, size);
	} else {
		off_t first_size = buffer_size - buffer_offset;

		memcpy(buf, buffer+buffer_offset, first_size);
		memcpy(buf + first_size, buffer, size - first_size);
	}

	return size;
}

static int buffer_write(const char *path, const char *buf, size_t size,
		      off_t offset, struct fuse_file_info *fi)
{
	size_t original_size = size;
	/* We implement pure append and ignore offset */
	(void) offset;
	(void) fi;

	printf("receiving write on '%s' offset=%zu size=%zu\n", path, offset, size);
	if(strcmp(path, "/") != 0)
		return -ENOENT;

	/* Only write the last buffer_size bytes */
	if (size > buffer_size) {
		buf += size - buffer_size;
		size = buffer_size;
	}

	if (buffer_offset + size <= buffer_size) {
		memcpy(buffer+buffer_offset, buf, size);
	} else {
		off_t first_size = buffer_size - buffer_offset;

		memcpy(buffer+buffer_offset, buf, first_size);
		memcpy(buffer, buf + first_size, size - first_size);
		wrapped = 1;
	}

	printf("changing offset from %zu to %zu\n",
			buffer_offset, (buffer_offset + size) % buffer_size);
	buffer_offset = (buffer_offset + size) % buffer_size;

	return original_size;
}

static struct fuse_operations buffer_oper = {
	.getattr	= buffer_getattr,
	.truncate	= buffer_truncate,
	.open		= buffer_open,
	.read		= buffer_read,
	.write		= buffer_write,
};

static struct buffer_config {
     char *size;
} buffer_config;

#define BUFFER_OPT(t, p, v) { t, offsetof(struct buffer_config, p), v }

static struct fuse_opt buffer_opts[] = {
	BUFFER_OPT("size=%s", size, 0),
	FUSE_OPT_END
};

int setup_size(char *s)
{
	char *endptr;
	uintptr_t oldsize, size;
		
	oldsize = size = strtoull(s, &endptr, 0);

	switch (*endptr) {
		case '\0':
			break;

		case 'g':
			size *= 1000;
		case 'm':
			size *= 1000;
		case 'k':
			size *= 1000;
			endptr++;
			break;

		case 'G':
			size *= 1024;
		case 'M':
			size *= 1024;
		case 'K':
			size *= 1024;
			endptr++;
			break;
	}

	if (size < oldsize) {
		fprintf(stderr, "integer overflow\n");
		return 1;
	}
	if (*endptr != '\0') {
		fprintf(stderr, "invalid number string '%s'\n", s);
		return 1;
	}

	buffer_size = size;
	return 0;
}

int main(int argc, char *argv[])
{
	struct fuse_args tmp_args, args = FUSE_ARGS_INIT(argc, argv);
	struct fuse_cmdline_opts opts;
	struct stat stbuf;

	tmp_args = args;
	if (fuse_parse_cmdline(&tmp_args, &opts) != 0)
		return 1;
	fuse_opt_free_args(&tmp_args);

	if (!opts.mountpoint) {
		fprintf(stderr, "missing mountpoint parameter\n");
		return 1;
	}

	if (stat(opts.mountpoint, &stbuf) == -1) {
		fprintf(stderr ,"failed to access mountpoint %s: %s\n",
			opts.mountpoint, strerror(errno));
		free(opts.mountpoint);
		return 1;
	}
	free(opts.mountpoint);
	if (!S_ISREG(stbuf.st_mode)) {
		fprintf(stderr, "mountpoint is not a regular file\n");
		return 1;
	}

	if (fuse_opt_parse(&args, &buffer_config, buffer_opts, NULL) == -1) {
		return 1;
	}

	if (buffer_config.size && setup_size(buffer_config.size)) {
		return 1;
	}

	buffer = calloc(1, buffer_size);
	if (!buffer) {
		perror("calloc");
		return 1;
	}

	return fuse_main(args.argc, args.argv, &buffer_oper, NULL);
}
