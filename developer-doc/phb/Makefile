all: index.html phb.pdf

SRC_FILES= phb.docbook cfg-mgmt.docbook coding.docbook \
	cvs-examples.docbook  src-examples.docbook \
	licence.docbook

index.html:	$(SRC_FILES)
	-docbook2html --cat /usr/share/sgml/CATALOG.docbk41 phb.docbook

phb.pdf:	$(SRC_FILES)
	-docbook2pdf --cat /usr/share/sgml/CATALOG.docbk41 phb.docbook
# somewhere in the doc to docbook2x tools, I found that pdfjadetex needs
# to be started up to three times to resolve all references. Since we
# don't know if we need it or not, we just do that. And it helps!
	pdfjadetex phb.tex
	pdfjadetex phb.tex
	pdfjadetex phb.tex
	
clean:
	-rm *.html *.pdf *.out *.tex *.log *.aux
