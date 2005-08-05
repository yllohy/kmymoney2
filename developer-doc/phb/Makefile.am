KDE_OPTIONS = noautodist

DOCBOOK_FILES = phb.docbook cfg-mgmt.docbook coding.docbook cvs-examples.docbook  src-examples.docbook licence.docbook error-mgmt.docbook unit-test.docbook rel-mgmt.docbook test-examples.docbook rpm-example.docbook fdl-licence

EXTRA_DIST = $(DOCBOOK_FILES)

MOSTLYCLEANFILES = *.html *.pdf *.ps phb.docbook.tex

TARGETS = phb.html

if HAVE_DSSSLCONVERSIONTOOLS
PDF_TARGET = kmymoney-phb.pdf
else
if HAVE_HTMLCONVERSIONTOOLS
PDF_TARGET = kmymoney-phb.pdf
endif
endif

.PHONY: all
all:	$(TARGETS) $(PDF_TARGET)

STYLESHEET = kde-nochunk.xsl

phb.html:	$(DOCBOOK_FILES)
	@if test -e $(MEINPROC); then \
		echo $(MEINPROC) --stylesheet $(kde_libs_prefix)/share/apps/ksgmltools2/customization/$(STYLESHEET) $(srcdir)/phb.docbook -o phb.html; \
		$(MEINPROC) --stylesheet $(kde_libs_prefix)/share/apps/ksgmltools2/customization/$(STYLESHEET) $(srcdir)/phb.docbook -o phb.html; \
	fi


if HAVE_DSSSLCONVERSIONTOOLS
DOCBOOK_DSL = /usr/share/sgml/docbook/dsssl-stylesheets/print/docbook.dsl

# if we're in a VPATH environment, we need to copy all files over
# to the build directory. Otherwise, we'll get into trouble. Of course
# we delete them afterwards.
phb.docbook.tex: $(DOCBOOK_FILES)
	if test x$(VPATH) != x.; then \
		for file in $(DOCBOOK_FILES); do \
			cp $(srcdir)/$$file .; \
		done; \
	fi
	-jade -t tex -d $(DOCBOOK_DSL) -D `dirname $(KDE_XSL_STYLESHEET)` -D $(kde_libs_htmldir)/en/common phb.docbook
	if test x$(VPATH) != x.; then \
		for file in $(DOCBOOK_FILES); do rm $$file; done;  \
	fi

kmymoney-phb.pdf: phb.docbook.tex
	@echo "Generating PDF document"
	@pdfjadetex phb.docbook.tex > errorlog
	@echo "Generating bookmarks"
	@pdfjadetex phb.docbook.tex >> errorlog
	@echo "Generating page numbers for table of contents"
	@pdfjadetex phb.docbook.tex >> errorlog
	mv phb.docbook.pdf kmymoney-phb.pdf
	rm phb.docbook.aux phb.docbook.log phb.docbook.out errorlog
else
if HAVE_HTMLCONVERSIONTOOLS

kmymoney-phb.pdf: phb.html
	html2ps -D -t phb.html | ps2pdf14 - phb.pdf
	mv phb.pdf kmymoney-phb.pdf
endif
endif
