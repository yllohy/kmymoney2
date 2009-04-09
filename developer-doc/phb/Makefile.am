KDE_OPTIONS = noautodist

DOCBOOK_FILES = phb.docbook cfg-mgmt.docbook coding.docbook cvs-examples.docbook  src-examples.docbook licence.docbook error-mgmt.docbook unit-test.docbook rel-mgmt.docbook test-examples.docbook rpm-example.docbook fdl-licence translation.docbook dialogs.docbook documentation.docbook settings.docbook submissions.docbook

EXTRA_DIST = $(DOCBOOK_FILES)

MOSTLYCLEANFILES = *.html *.pdf *.ps phb.docbook.tex online-phb.tar.bz2

TARGETS = phb.html

if GENERATE_PDF
PDF_TARGET = kmymoney-phb.pdf
endif

.PHONY: all show preview get-files clear-files web
all:	$(TARGETS) $(PDF_TARGET)

STYLESHEET = kde-nochunk.xsl

phb.html:	$(DOCBOOK_FILES)
	@if test -e $(MEINPROC); then \
		echo $(MEINPROC) --stylesheet $(kde_libs_prefix)/share/apps/ksgmltools2/customization/$(STYLESHEET) $(srcdir)/phb.docbook -o phb.html; \
		echo sed -i "s/ISO-8859-1/UTF-8/g" phb.html; \
		$(MEINPROC) --stylesheet $(kde_libs_prefix)/share/apps/ksgmltools2/customization/$(STYLESHEET) $(srcdir)/phb.docbook --stdout | sed "s/ISO-8859-1/UTF-8/g" > phb.html; \
	fi

preview: phb.html

show: preview
	konqueror phb.html

# if we're in a VPATH environment, we need to copy all files over
# to the build directory. Otherwise, we'll get into trouble. Of course
# we delete them afterwards.
get-files:
	if test -n "$(VPATH)" ; then \
		if test x$(VPATH) != x.; then \
			for file in $(DOCBOOK_FILES) $(SCREENSHOT_FILES); do \
				cp $(srcdir)/$$file .; \
			done; \
		fi \
	fi

clear-files:
	if test -n "$(VPATH)" ; then \
		if test x$(VPATH) != x.; then \
			for file in $(DOCBOOK_FILES) $(SCREENSHOT_FILES); do rm $$file; done;  \
		fi \
	fi


if GENERATE_PDF
web: $(PDF_TARGET)
	if test x`which splitup-kde-chunk-online`x != xx; then \
		rm -rf phb common; \
		mkdir phb; \
		mkdir common; \
		$(MEINPROC) --stylesheet `dirname $(KDE_XSL_STYLESHEET)`/kde-chunk-online.xsl  $(srcdir)/phb.docbook -o phb/index.xml; \
		cd phb; \
		splitup-kde-chunk-online; \
		rm index.xml; \
		cd ..; \
		make get-files; \
		for file in $(SCREENSHOT_FILES); do \
			cp $$file phb; \
		done; \
		make clear-files; \
		cp $(kde_libs_htmldir)/en/common/* common; \
		if test ! -z "$(PDF_TARGET)"; then \
			cp $(PDF_TARGET) phb; \
		fi; \
		sed -i s/help:/../g common/kde-default.css; \
		chmod 664 phb/* common/*; \
		tar -cjf online-phb.tar.bz2 common phb; \
		rm -rf index.xml phb common; \
	fi;

# if we're in a VPATH environment, we need to copy all files over
# to the build directory. Otherwise, we'll get into trouble. Of course
# we delete them afterwards.

kmymoney-phb.pdf: $(DOCBOOK_FILES)
	$(MAKE) get-files
	$(MEINPROC) --stylesheet `dirname $(KDE_XSL_STYLESHEET)`/kde-nochunk.xsl $(srcdir)/phb.docbook -o index.html
	recode utf8..latin1 index.html
	html2ps -o kmymoney-phb.ps -n index.html
	ps2pdf kmymoney-phb.ps kmymoney-phb.pdf
	rm index.html kmymoney-phb.ps
	$(MAKE) clear-files
endif

