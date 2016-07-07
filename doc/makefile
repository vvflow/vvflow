#packages needed:
#texlive-lang-cyrillic, texlive-latex-extra, texlive-publishers, cm-super

# Несколько сценариев построения картинок
#1  imgsrc/gnuplot --gp--> images/svg ------> images/pdf + images/pdf_tex
#2  imgsrc/svg     --ln--> images/svg ------> images/pdf + images/pdf_tex
#3  imgsrc/png     --ln--> images/png

all_images :=
pdf = $(patsubst %, images/%.pdf_tex, $(1))
png = $(patsubst %, images/%.png, $(1))

all_images += $(call pdf, RefFrames)
all_images += $(call pdf, ConvInfluence)
all_images += $(call pdf, VortexDiff BodyDiff)

all: $(all_images)
	xelatex documentation.tex
	rm -f *.aux *.log *.out *.toc

clean:
	rm -rf images/ *.log *.aux

images/:
	mkdir -p $@

# ln as is
vpath %.pdf_tex $(dir $(wildcard imgsrc/*/))
images/%.pdf_tex: %.pdf_tex | images/
	@git ls-files $< --error-unmatch > /dev/null
	ln -f $< $@
vpath %.svg $(dir $(wildcard imgsrc/*/))
images/%.svg: %.svg | images/
	@git ls-files $< --error-unmatch > /dev/null
	ln -f $< $@
vpath %.png $(dir $(wildcard imgsrc/*/))
images/%.png: %.png | images/
	@git ls-files $< --error-unmatch > /dev/null
	ln -f $< $@

# gnuplot to svg
vpath %.gnuplot $(dir $(wildcard imgsrc/*/))
images/%.svg: %.gnuplot | images/
	@git ls-files $< --error-unmatch > /dev/null
	@git ls-files $(addprefix $(dir $<),$(shell grep -oP '\w+\.data' $< | sort | uniq)) --error-unmatch > /dev/null
	GNUPLOT_LIB=$(dir $<) $< > $@

# svg to pdf_tex
images/%.pdf_tex: images/%.svg | images/
	inkscape -z --file=$< --export-pdf=$(patsubst %.pdf_tex,%.pdf,$@) --export-latex

# svg to pdf
images/%_svg.pdf: images/%.svg | images/
	inkscape -z --file=$< --export-pdf=$@
