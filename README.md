# NCSL

Ncsl counts the lines of code in a C or C++ program, including:

* the number of raw lines of code (sloc)
* the number of non-comment, non-blank lines (ncsl)
* the number of comments

and optionally, with option -v:

* the number of statement separators (semicolons and commas)

The output is by default generated in Unix-style, giving the three or four fields
defined above per line, similar to the output from `wc`.

If you specify no files ncsl reads from stdin.
If you specify more than one file on the command line, `ncsl`
prints the total for each category.

### Options

Option -n show stripped output with line numbers.

Option -r modifies -n, using line numbers from the _input_ file.

Option -s shows stripped output without linenumbers.

Option -u produces cumulative output for all files, with
one annotated field per line, instead of the default `wc` like output.

Option -v count also semicolons and commas, and add a ratio ncsl:sloc to the -u output.

Option -V prints the `ncsl` version number and exits.

### EXAMPLES

* `ncsl *.c`

* `ncsl -s ncsl.c`

* `ncsl -n -r ncsl.c`

* `ncsl -u -v ncsl.c`

* `cat ncsl.c | ncsl`

* `cat *.c */*.c | ncsl`
