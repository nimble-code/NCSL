ncsl: ncsl.c
	cc -Wall -pedantic -o ncsl ncsl.c

install: ncsl
	cp ncsl /usr/local/bin

clean:
	rm -f ncsl

