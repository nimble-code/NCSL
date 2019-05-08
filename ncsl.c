#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/** simple comment filter and line counter -- gjh 2004, revised 2019 **/

enum cstate { PLAIN, IN_STRING, IN_QUOTE, S_COMM, COMMENT, CPP_COMMENT, E_COMM };

static enum cstate state = PLAIN;
static char buf[8192];
static int column, nc_linecount, n_comments;
static int raw, relative, verbose, unix_style = 1;
static int numbered, showlines, lastadded;
static int semi_cnt, t_semi_cnt;
static int sum_rw, sum_nc, sum_cm, sum_sm;

int max_seen;

static void
add_buf(int c)
{
	if (column < sizeof(buf))
	{	buf[column] = c;
		column++;
	}
	lastadded = c;
}

static void
semi_check(char *b)
{	char *s;
	unsigned char saw_semilast = 0;

	for (s = b; *s != '\0'; s++)
	{	switch (*s) {
		case ';':
		case ',':
			saw_semilast = 1;
			semi_cnt++;
			break;
		case ' ':
		case '\t':
		case '\f':
		case '\v':
		case '\r':
			break;
		default:
			saw_semilast = 0;
			break;
	}	}
	if (saw_semilast)
	{	t_semi_cnt++;
	}	
}

static void
process_line(char *filename)
{	int i;

	if (state == PLAIN)
	{	if (column > 0)
		{	for (i = 0; i < column; i++)
			{	if (!isspace(buf[i]))
				{	break;
			}	}
			if (i == column)
			{	goto emptyline;
			}
			if (column >= sizeof(buf) && showlines)
			{	fprintf(stderr, "%s:%d, error: line too long\n",
					filename, relative);
			} else
			{	buf[column] = '\0';
				if (verbose)
				{	semi_check(&buf[i]);
				}
				if (showlines)
				{	if (numbered)
					{	printf("%6d: ", nc_linecount+1);
					}
					printf("%s\n", buf);
			}	}
			nc_linecount++;
		} else
		{
emptyline:		if (showlines)
			{	printf("\n");
		}	}
	}
	column = lastadded = 0;
	raw++; relative++;
}

static void
process_file(FILE *fd, char *filename, ...)
{	int c, start_delim = 0;

	while ((c = getc(fd)) != EOF)
	{	if (c == '\r')
		{	continue;
		}
		switch (state) {
		case PLAIN:
			switch (c) {
			case  '"': state = IN_STRING; break;
			case '\'': state = IN_QUOTE; break;
			case  '/': state = S_COMM; start_delim = column; break;
			case '\\':
				c = getc(fd);
same:				add_buf('\\');
				break;
			}
			break;
		case IN_STRING:
			switch (c) {
			case '"': state = PLAIN; break;
			case '\\': c = getc(fd); goto same;
			default:  break;
			}
			break;
		case IN_QUOTE:
			switch (c) {
			case '\'': state = PLAIN; break;
			case '\\': c = getc(fd); goto same;
			default:   break;
			}
			break;
		case S_COMM:
			switch (c) {
			case '*': state = COMMENT;
				  n_comments++;
				  column = start_delim-1;
				  break;
			case '/': state = CPP_COMMENT;
				  n_comments++;
				  column = start_delim-1;
				  /* nc_linecount--; */
				  break;		/* C++ style comment */
			default:  state = PLAIN;
				  break;
			}
			break;
		case COMMENT:
			switch (c) {
			case '*':
				state = E_COMM;
				/* fall through */
			default: 
				break;
			}
			break;
		case E_COMM:
			switch (c) {
			case '/': state = PLAIN; continue;
			case '*': break;
			default:  state = COMMENT; break;
			}
			break;
		case CPP_COMMENT:
			if (c == '\n')
			{	state = PLAIN;
			}
			break;
		default:
			break;
		}
		if (c == '\n')
		{	process_line(filename);
		} else if (state != COMMENT && state != CPP_COMMENT && state != E_COMM)
		{	add_buf(c);
	}	}
	return;
}

int
main(int argc, char *argv[])
{	FILE *fd = stdin;
	int nrfiles = 0;

	while (argc > 1 && argv[1][0] == '-')
	{	switch (argv[1][1]) {
		case 'n':
			numbered = 1;
			/* fall through */
		case 's':
			showlines = 1;
			break;
		case 'u':
			unix_style = 1 - unix_style;
			break;
		case 'v':
			verbose++;
			break;
		default:
			printf("usage: ncsl [-n] [-s] [-m] *.c\n");
			printf("by default, wc-like output with 3 or 4 fields\n");
			printf("	field 1: nr of raw lines of input\n");
			printf("	field 2: nr of non-blank/non-comment lines\n");
			printf("	field 3: nr comments\n");
			printf("	field 4: nr of semi-colons and commas\n");
			printf("options:\n");
			printf(" -n show stripped output with numbered lines\n");
			printf(" -s show stripped output\n");
			printf(" -u show results one field per line, cumulative\n");
			printf(" -v add the 4th field of output\n");
			exit(1);
		}
		argv++; argc--;
	}

	if (unix_style && !showlines)
	{	printf("    sloc     ncsl comments");
		if (verbose)
		{	printf("       ;,");
		}
		if (argc > 1)
		{	printf("   file");
		}
		printf("\n");
	}

	if (argc == 1)
	{	process_file(fd, "<stdin>");
		if (showlines)
		{	printf("\n");
		} else if (unix_style)
		{	printf("%8d %8d %8d",
				raw, nc_linecount, n_comments);
			if (verbose)
			{	printf(" %8d", semi_cnt);
			}
			printf("\n");
		}
		sum_rw = raw;
		sum_nc = nc_linecount;
		sum_cm = n_comments;
		sum_sm = semi_cnt;
	} else
	{	while (argc > 1)
		{	fd = fopen(argv[1], "r");
			if (fd == NULL)
			{	fprintf(stderr, "ncsl: cannot open %s\n", argv[1]);
				argv++; argc--;
				continue;
			}

			nrfiles++;
			relative = 0;
			state = PLAIN;
			process_file(fd, argv[1]);
			fclose(fd);

			if (showlines)
			{	printf("\n");
			} else if (unix_style)
			{	printf("%8d %8d %8d",
					raw, nc_linecount, n_comments);
				if (verbose)
				{	printf(" %8d ", semi_cnt);
				}
				printf("  %s\n", argv[1]);
			}
			sum_rw += raw; raw = 0;
			sum_nc += nc_linecount; nc_linecount = 0;
			sum_cm += n_comments; n_comments = 0;
			sum_sm += semi_cnt; semi_cnt = 0;
			argv++; argc--;
	}	}


	if (unix_style)
	{	if (nrfiles > 1)
		{	printf("%8d %8d %8d",
				sum_rw, sum_nc, sum_cm);
			if (verbose)
			{	printf(" %8d", sum_sm);
			}
			printf("  total \t[ncsl:sloc %6d%%]\n", (100*sum_nc)/(sum_rw+1));
		}
	} else
	{	printf("SLOC: %6d\t(raw lines of source code)\n", sum_rw);
		printf("NCSL: %6d\t(non-comment, non-blank lines of code)\n", sum_nc);
		printf("CMNT: %6d\t(nr of comments)\n", sum_cm);
		if (verbose) {
		printf("SSEPS:%6d\t(Statement separators: semicolons and commas)\n", sum_sm);
		printf("Ratio:%6d%%\t(NCSL : SLOC)\n", (100*sum_nc)/(sum_rw+1));
	/*	printf("TSEMI:%6d\t(Semicolons terminating a line)\n\n", t_semi_cnt); */
		}
	}
	return 0;
}
