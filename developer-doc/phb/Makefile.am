####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = cfg-mgmt.docbook coding.docbook cvs-examples.docbook error-mgmt.docbook fdl-licence licence.docbook phb.docbook src-examples.docbook unit-test.docbook rel-mgmt.docbook test-examples.docbook 

####### kdevelop will overwrite this part!!! (end)############

DISTCLEANFILES = cfg-mgmt.html coding-std.html create-stable-example.html cvs-checkin.html cvs-examples.html cvs-update.html dismissing-changes.html header-files.html index.html licence.html multiple-branches.html problem-attributes.html problem-mgmt.html promoting-changes.html rel-mgmt.html repo-access.html rpm.html source-example.html source-files.html src-examples.html test-container-example.html test-examples.html test-source-example.html unit-test.html unit-testing-howto.html unit-testing-in-kmm.html version-controlled-files.html version-mgmt.html phb.aux phb.log phb.out phb.pdf phb.tex

CLEAN_FILES = *.html *.pdf *.out *.tex *.log *.aux

SRC_FILES= phb.docbook cfg-mgmt.docbook coding.docbook \
	cvs-examples.docbook  src-examples.docbook \
	licence.docbook error-mgmt.docbook \
	unit-test.docbook rel-mgmt.docbook \
	test-examples.docbook

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
