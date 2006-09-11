KDE_OPTIONS = noautodist

DOCBOOK_FILES = phb.docbook cfg-mgmt.docbook coding.docbook cvs-examples.docbook  src-examples.docbook licence.docbook error-mgmt.docbook unit-test.docbook rel-mgmt.docbook test-examples.docbook rpm-example.docbook fdl-licence translation.docbook dialogs.docbook documentation.docbook

EXTRA_DIST = $(DOCBOOK_FILES)

MOSTLYCLEANFILES = *.html *.pdf *.ps phb.docbook.tex online-phb.tar.bz2

TARGETS = phb.html

if HAVE_DOCCONVERSIONTOOLS
PDF_TARGET = kmymoney-phb.pdf
endif

.PHONY: all get-files clear-files web
all:	$(TARGETS) $(PDF_TARGET)

STYLESHEET = kde-nochunk.xsl

phb.html:	$(DOCBOOK_FILES)
	@if test -e $(MEINPROC); then \
		echo $(MEINPROC) --stylesheet $(kde_libs_prefix)/share/apps/ksgmltools2/customization/$(STYLESHEET) $(srcdir)/phb.docbook -o phb.html; \
		echo sed -i "s/ISO-8859-1/UTF-8/g" phb.html; \
		$(MEINPROC) --stylesheet $(kde_libs_prefix)/share/apps/ksgmltools2/customization/$(STYLESHEET) $(srcdir)/phb.docbook --stdout | sed "s/ISO-8859-1/UTF-8/g" > phb.html; \
	fi

# if we're in a VPATH environment, we need to copy all files over
# to the build directory. Otherwise, we'll get into trouble. Of course
# we delete them afterwards.
get-files:
	if test x$(VPATH) != x.; then \
		for file in $(DOCBOOK_FILES) $(SCREENSHOT_FILES); do \
			cp $(srcdir)/$$file .; \
		done; \
	fi

clear-files:
	if test x$(VPATH) != x.; then \
		for file in $(DOCBOOK_FILES) $(SCREENSHOT_FILES); do rm $$file; done;  \
        fi


if HAVE_DOCCONVERSIONTOOLS
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

kmymoney-phb.pdf: $(DOCBOOK_FILES)
	$(MAKE) get-files
	@dblatexbin@ -b pdftex --style kdestyle -o kmymoney-phb.pdf \
		-P latex.output.revhistory=0  -P newtbl.use=1 \
		-P imagedata.default.scale=pagebound \
		-P literal.width.ignore=1 \
		-I $(kde_libs_htmldir)/en \
		-X \
		phb.docbook
	$(MAKE) clear-files
endif
