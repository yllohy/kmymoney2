all: index.html phb.pdf

index.html:	phb.docbook
	-docbook2html --cat /usr/share/sgml/CATALOG.docbk41 phb.docbook

phb.pdf:	phb.docbook
	-docbook2pdf --cat /usr/share/sgml/CATALOG.docbk41 phb.docbook
	pdfjadetex phb.tex
	
clean:
	-rm *.html *.pdf *.out *.tex *.log *.aux
