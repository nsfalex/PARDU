CC=clang
CFLAGS=-Wall -O2 # -std=c99 
LDFLAGS=-lc -lncurses -lpthread
BINNAME=pardu
OUTDIR=out

all: prep build

.PHONY: prep
prep: $(OUTDIR)

$(OUTDIR):
	mkdir $@


.PHONY: build
build: $(OUTDIR)/$(BINNAME)

$(OUTDIR)/$(BINNAME): prep
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) src/*.c

.PHONY: clean
clean:
	rm -rf $(OUTDIR)/*
