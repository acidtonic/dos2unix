/*
 *                              DOS2UNIX.C
 *
 * Clean out cr/lf combinations in a file but keep it's original
 * date/time stamp.
 */

#include <ftw.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <utime.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef TRUE
#	define TRUE  (1)
#	define FALSE (0)
#endif

#define R_CNTRL   "r"
#define W_CNTRL   "w"



struct stat s_buf;

int dos2u (char *path);
int dos2uFolder (char *path);

int main (int argc, char **argv)
{
	char *path;
	int c;
	bool recurse = false;
	while ((c = getopt (argc, argv, "hrv")) != -1)
	{
		switch (c)
		{
			case 'h':
				printf ("Usage: [-h | -v] [-r] file [file, ...]\n");
				printf ("Converts \\r\\n to \\n in files\n");
				printf ("  -h: Get the help (this one!)\n");
				printf ("  -v: Get the version\n");
				printf ("  -r: Get files recursive\n");
				exit (0);
				break;
			case 'v':
				printf ("Version: 1.0.1\n");
				printf ("Original: Unknown, Modified by :Puck Meerburg.\n");
				exit (0);
				break;
			case 'r':
				recurse = true;
				break;
		}
	}

	while (optind < argc)
	{
		if (stat (path=argv[optind], &s_buf) == -1)
		{
			fprintf (stderr, "Dos2Unix: Can't stat '%s'\n", path);
		}

		if(s_buf.st_mode & S_IFDIR)
		{
			if (recurse)
				dos2uFolder (path);
		}
		else
		{
			dos2u (path);
		}

		optind++;
	}
}





int dos2u (char *path)
{
	printf ("Dos2Unix: Cleaning file %s ...\n", path);
	FILE *in, *out;
	int ch,
	    rval = FALSE;
	char temppath [16];
/*	struct utimbuf { time_t actime, modtime; } ut_buf; */
	struct utimbuf ut_buf;

	strcpy (temppath, "./clntmp");
	if ((in=fopen (path, R_CNTRL)) == (FILE *) 0)
		return TRUE;
	if ((out=fopen (temppath, W_CNTRL)) == (FILE *) 0)
	{
		fclose (in);
		return TRUE;
	}
	while ((ch = getc (in)) != EOF)
		if ((ch != '\015' && ch != '\032') &&
			(putc (ch, out) == EOF)           )
		{
			rval = TRUE;
			break;
		}
	if (fclose (in) == EOF)
	{
		rval = TRUE;
	}
	if (fclose (out) == EOF)
	{
		rval = TRUE;
	}
	ut_buf.actime = s_buf.st_atime;
	ut_buf.modtime = s_buf.st_mtime;
	if (utime (temppath, &ut_buf) == -1)
		rval = TRUE;
	if (unlink (path) == -1)
		rval = TRUE;
	if (rval)
	{
		unlink (temppath);
		return TRUE;
	}
	if (rename (temppath,path) == -1)
	{
		fprintf (stderr, "Dos2Unix: Problems renaming '%s' to '%s'\n", temppath, path);
		fprintf (stderr, "          However, file '%s' remains\n", temppath);
		exit (1);
	}
	unlink (temppath);
	return FALSE;
}

int loop(const char *path, const struct stat *stat, int type)
{
	if(type == FTW_F)
		if (dos2u ((char *)path))
				fprintf (stderr, "Dos2Unix: Problems cleaning file %s\n", path);
	return 0;
}

int dos2uFolder (char *path)
{
	printf ("Dos2Unix: Cleaning directory %s ... \n", path);
	return ftw (path, &loop, OPEN_MAX) == 0;
}
