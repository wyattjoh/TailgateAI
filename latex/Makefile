JOBNAME = cmput350-paper

# You want latexmk to *always* run, because make does not have all the info.
.PHONY: $(JOBNAME).pdf

# First rule should always be the default "all" rule, so both "make all" and
# "make" will invoke it.
all: $(JOBNAME).pdf

# CUSTOM BUILD RULES

# In case you didn't know, '$@' is a variable holding the name of the target,
# and '$<' is a variable holding the (first) dependency of a rule.

%.tex: %.raw
	./raw2tex $< > $@

%.tex: %.dat
	./dat2tex $< > $@

# MAIN LATEXMK RULE

# -pdf tells latexmk to generate PDF directly (instead of DVI).
# -pdflatex="" tells latexmk to call a specific backend with specific options.
# -use-make tells latexmk to call make for generating missing files.

# -interactive=nonstopmode keeps the pdflatex backend from stopping at a
# missing file reference and interactively asking you for an alternative.

preview: all
	open $(JOBNAME).pdf

$(JOBNAME).pdf: $(JOBNAME).tex $(JOBNAME)-bibliography.bib
	latexmk -pdf -bibtex -pdflatex="pdflatex" -use-make $(JOBNAME).tex

clean:
	latexmk -c
