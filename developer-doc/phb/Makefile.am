KDE_OPTIONS = noautodist

DOCBOOK_FILES = phb.docbook cfg-mgmt.docbook coding.docbook cvs-examples.docbook  src-examples.docbook licence.docbook error-mgmt.docbook unit-test.docbook rel-mgmt.docbook test-examples.docbook rpm-example.docbook

EXTRA_DIST = $(DOCBOOK_FILES)

MOSTLYCLEANFILES = *.html 

phb:	phb.html

STYLESHEET = kde-nochunk.xsl

phb.html:	$(DOCBOOK_FILES)
	@if test -e $(MEINPROC); then \
		echo $(MEINPROC) --stylesheet $(kde_datadir)/ksgmltools2/customization/$(STYLESHEET) $(srcdir)/phb.docbook -o phb.html; \
		$(MEINPROC) --stylesheet $(kde_datadir)/ksgmltools2/customization/$(STYLESHEET) $(srcdir)/phb.docbook -o phb.html; \
	fi


