## If there are sub-documents, put them in a sub-directory with the same name,
## i.e. `mqtt-codec.tex` would include `mqtt-codec/performance.tex`.

DOCUMENTS := $(wildcard *.tex)
OUTPUT := $(DOCUMENTS:.tex=.pdf)

all: $(OUTPUT)


%.pdf: %.tex
	latexmk -pdf -use-make $<

clean:
	latexmk -quiet -CA
