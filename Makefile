
.POSIX:
.SUFFIXES:

.PHONY: all clean

CFLAGS = -std=c99 -Wall -Wextra -Wno-unused-parameter -g -Os

SEXP2JSON_IN = $(wildcard test/sexp2json/*.sexp)
SEXP2JSON_OUT = $(SEXP2JSON_IN:%.sexp=%.json)

JSON2SEXP_IN = $(wildcard test/json2sexp/*.json)
JSON2SEXP_OUT = $(JSON2SEXP_IN:%.json=%.sexp)

all: json2sexp sexp2json $(SEXP2JSON_OUT) $(JSON2SEXP_OUT)

clean:
	rm -f json2sexp sexp2json

json2sexp: json2sexp.c lisp.h
	cc $(CFLAGS) -o $@ $<

sexp2json: sexp2json.c lisp.h
	cc $(CFLAGS) -o $@ $<

test/sexp2json/%.json: test/sexp2json/%.sexp sexp2json Makefile
	./sexp2json < $< > $@

test/json2sexp/%.sexp: test/json2sexp/%.json json2sexp Makefile
	./json2sexp < $< > $@
