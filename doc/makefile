#packages needed:
#texlive-lang-cyrillic, texlive-latex-extra, texlive-publishers, cm-super

images_names := BodyDiff ConvInfluence RefFrames
images_pdf := $(patsubst %, images/%.pdf, $(images_names))

all: $(images_pdf)
	pdflatex documentation.tex

%.pdf: %.svg
	inkscape -z --file=$^ --export-pdf=$@ --export-latex

clean:
	rm -f *.log *.aux

clean_images:
	rm -f $(images_pdf) images/*.pdf_tex
