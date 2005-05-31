KDE_OPTIONS = noautodist

DOCBOOK_FILES = phb.docbook cfg-mgmt.docbook coding.docbook cvs-examples.docbook  src-examples.docbook licence.docbook error-mgmt.docbook unit-test.docbook rel-mgmt.docbook test-examples.docbook rpm-example.docbook

EXTRA_DIST = $(DOCBOOK_FILES)

MOSTLYCLEANFILES = *.html *.pdf *.ps

TARGETS = phb.html

if HAVE_HTML2PS
if HAVE_PS2PDF
TARGETS += phb.pdf
endif
endif

.PHONY: all
all:	$(TARGETS)

STYLESHEET = kde-nochunk.xsl

phb.html:	$(DOCBOOK_FILES)
	@if test -e $(MEINPROC); then \
		echo $(MEINPROC) --stylesheet $(kde_datadir)/ksgmltools2/customization/$(STYLESHEET) $(srcdir)/phb.docbook -o phb.html; \
		$(MEINPROC) --stylesheet $(kde_datadir)/ksgmltools2/customization/$(STYLESHEET) $(srcdir)/phb.docbook -o phb.html; \
	fi

phb.pdf: phb.html
	html2ps -D -t phb.html | ps2pdf14 - phb.pdf
