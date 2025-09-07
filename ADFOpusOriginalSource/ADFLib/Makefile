# General Makefile for ADFlib files

LIBDIR=Lib
TESTDIR=Test
DEMODIR=Demo
LIBNAME=libadf.a

all: tests lib demo

lib: $(LIBDIR)/$(LIBNAME)
	cp $(LIBDIR)/$(LIBNAME) ./Bin/Linux/

$(LIBDIR)/$(LIBNAME):
	cd $(LIBDIR) && $(MAKE)

clean:
	cd $(LIBDIR) && ($(MAKE) clean || true)
	cd $(TESTDIR) && ($(MAKE) clean || true)
	cd $(DEMODIR) && ($(MAKE) clean || true)

zip:
	($(MAKE) clean) || true
	(rm adflib.zip) || true
	zip -9r adflib.zip Lib Docs Faq Bin/Linux Boot Demo Makefile README CHANGES *.dsp *.dsw

dist:	zip	
	cd Dist && dist.sh

zipdump:
	(rm adfdump.zip) || true
	zip -9r dumps.zip Dumps Test

binw32:
	(rm binw32.zip) || true
	zip -9r binw32.zip Bin/Win32

backup:
	cp -f adflib.zip dumps.zip /mnt/winc/Laurent/

demo: $(LIBDIR)/$(LIBNAME)
	cd $(DEMODIR) &&  $(MAKE)


tests: $(LIBDIR)/$(LIBNAME)
	cd $(TESTDIR) && $(MAKE)

	echo
	echo "floppy tests"
	cd $(TESTDIR) && floppy.sh

	echo 
	echo "big devices tests"
	cd $(TESTDIR) && bigdev.sh


