
.POSIX:
.SUFFIXES:

.PHONY: all clean

CFLAGS = -std=c99 -Wall -Wextra -Wno-unused-parameter -g -Os

all: json2sexp sexp2json test/sexp2json/hello.json

clean:
	rm -f json2sexp sexp2json

json2sexp: json2sexp.c lisp.h
	cc $(CFLAGS) -o $@ $<

sexp2json: sexp2json.c lisp.h
	cc $(CFLAGS) -o $@ $<

test/sexp2json/%.json: test/sexp2json/%.sexp sexp2json Makefile
	./sexp2json < $< > $@
