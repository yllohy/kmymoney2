KDE_OPTIONS = noautodist

####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = cfg-mgmt.docbook coding.docbook cvs-examples.docbook error-mgmt.docbook fdl-licence licence.docbook phb.docbook src-examples.docbook unit-test.docbook rel-mgmt.docbook test-examples.docbook rpm-example.docbook 

####### kdevelop will overwrite this part!!! (end)############

DB2HTML=/usr/bin/db2html
DB2PDF=/usr/bin/db2pdf

DISTCLEANFILES = *.html phb.aux phb.log phb.out phb.pdf phb.tex

CLEAN_FILES = *.html *.pdf *.out *.tex *.log *.aux

SRC_FILES= phb.docbook cfg-mgmt.docbook coding.docbook \
	cvs-examples.docbook  src-examples.docbook \
	licence.docbook error-mgmt.docbook \
	unit-test.docbook rel-mgmt.docbook \
	test-examples.docbook rpm-example.docbook

phb:	index.html phb.pdf

phb-clean:
	-rm -f $(CLEAN_FILES)
	-rm -rf phb

#####################################################################
#####################################################################
#
# The rules below are heavily depending on the distribution
# The ones contained here work for SuSE9.1.  Since I had
# to change them multiple times for new distro releases,
# I assume that they will change in the future.
#
# As I said, they work for me, but probably will fail if
# you have a different environment. If you modify this
# file, please make sure you *DO NOT* check it in but
# rather keep a local copy. This way, we do not destroy
# our work. If you have a better idea how to deal with this
# please let me know. (Thomas Baumgart <ipwizard@users.sourceforge.net>)
#
#####################################################################
#####################################################################
.PHONY: index.html
index.html:	$(SRC_FILES)
	-if test -e $(DB2HTML); then \
	    $(DB2HTML) --catalog /usr/share/sgml/CATALOG.docbk41 -o phb -j "-o index.html" phb.docbook; \
	    ln -s book1.html phb/index.html; \
	fi

phb.pdf:	$(SRC_FILES)
# somewhere in the doc to docbook2x tools, I found that pdfjadetex needs
# to be started up to three times to resolve all references. Since we
# don't know if we need it or not, we just do that. And it helps!
#
# The latest SuSE (9.1) db2html script already has logic to do that
# but it does not work properly as the script terminates before. So
# we just do it ourselves.
	-if test -e $(DB2PDF); then \
	    ln -s phb.docbook phb.sgml; \
	    $(DB2PDF) --catalog /usr/share/sgml/CATALOG.docbk41 -j "-E 5000" phb.sgml; \
	    pdfjadetex phb.tex; \
	    pdfjadetex phb.tex; \
	    pdfjadetex phb.tex; \
	    rm phb.sgml; \
	fi
