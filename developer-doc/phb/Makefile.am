####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = cfg-mgmt.docbook coding.docbook cvs-examples.docbook error-mgmt.docbook fdl-licence licence.docbook phb.docbook src-examples.docbook 

####### kdevelop will overwrite this part!!! (end)############

CLEAN_FILES = *.html *.pdf *.out *.tex *.log *.aux

SRC_FILES= phb.docbook cfg-mgmt.docbook coding.docbook \
	cvs-examples.docbook  src-examples.docbook \
	licence.docbook error-mgmt.docbook

phb:	index.html phb.pdf

phb-clean:
	-rm -f $(CLEAN_FILES)

index.html:	$(SRC_FILES)
	-if test -e /usr/bin/docbook2html; then \
	    docbook2html --cat /usr/share/sgml/CATALOG.docbk41 phb.docbook; \
	fi

phb.pdf:	$(SRC_FILES)
# somewhere in the doc to docbook2x tools, I found that pdfjadetex needs
# to be started up to three times to resolve all references. Since we
# don't know if we need it or not, we just do that. And it helps!
	-if test -e /usr/bin/docbook2pdf; then \
	    docbook2pdf --cat /usr/share/sgml/CATALOG.docbk41 phb.docbook; \
	    pdfjadetex phb.tex; \
	    pdfjadetex phb.tex; \
	    pdfjadetex phb.tex; \
	fi
