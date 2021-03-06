#
# Makefile modified by David Hilvert to generate ALE documentation.
# 24-Sep-2006
#

# $Source$
# $Author: xmldoc $
# $Date: 2006-09-01 15:47:16 +0900 (Fri, 01 Sep 2006) $
# $Revision: 6235 $
# vim: number
#
# -----------------------------------------------------------------
#  ** Makefile.DocBook -- generate output from DocBook sources **
# -----------------------------------------------------------------
#
#   This file is part of the DocBook Project XSL Stylesheet
#   distribution.
#
#   See http://docbook.sourceforge.net/release/xsl/current/
#   for copyright and other information.

# DOCBOOK_OUTPUT_FORMATS is the default set of targets (output
# formats) that get built when you type "make" without any targets
# explicitly specified. To generate a different set of output
# formats, change the value of DOCBOOK_OUTPUT_FORMATS here or set
# it in your environment; for example:
#
#   set DOCBOOK_OUTPUT_FORMATS="html pdf"; export DOCBOOK_OUTPUT_FORMATS
#
# Of course by explicitly specifying particular targets when you
# invoke "make", you can always override generation of the default
# set of targets; for example:
#
#   make html txt
#
# That would generate just HTML (unchunked) and plain-text output.
#
DOCBOOK_OUTPUT_FORMATS ?= man chunk txt pdf info

# If you want XHTML output instead of HTML, set HTML_OR_XHTML to
# 'xhtml' or just specify 'xhtml" in DOCBOOK_OUTPUT_FORMATS.
ifeq ($(findstring xhtml,$(DOCBOOK_OUTPUT_FORMATS)),)
HTML_OR_XHTML ?= html
else
HTML_OR_XHTML ?= xhtml
endif

# -----------------------------------------------------------------
#                *** TOOLS and other DEPENDENCIES ***
# -----------------------------------------------------------------
# we use rmdir(1) to remove dirs we create for chunked HTML output
RMDIR = rmdir
# "-p" causes empty parent dirs to be deleted as well
RMDIR_FLAGS = --ignore-fail-on-non-empty -p

# possible values for PDF_MAKER are:
#   dblatex|fop|xep|xmlroff|passivetex
PDF_MAKER = fop

# possible values for TXT_MAKER are:
#   links|lynx|w3m|w3mmee
TXT_MAKER = links
# TXT_MAKER_FLAGS = -dump -no-numbering -no-references
TXT_MAKER_FLAGS = -dump

# xsl
XSLT = xsltproc
XSLT_FLAGS = --xinclude $(XSLT_VERSION_INFO)

# http://dblatex.sourceforge.net/
DBLATEX = dblatex
DBLATEX_FLAGS = -b pdftex -P page.margin.inner=0.8 -P page.margin.outer=0.5 -P double.sided=1 -T db2latex

FOP = fop
FOP_FLAGS =

XEP = xep
XEP_FLAGS =

# http://xmlroff.sourceforge.net/
XMLROFF = xmlroff
XMLROFF_FLAGS =

PDFTEX = pdftex
PDFTEX_FLAGS =

# used by PassiveTeX
PDFXMLTEX = pdfxmltex

# http://docbook2x.sourceforge.net/
DB2X_XSLTPROC = db2x_xsltproc
DB2X_XSLTPROC_FLAGS = --xinclude
DB2X_TEXIXML = db2x_texixml --list-files
DB2X_TEXIXML_FLAGS = 

# we call the man(1) command to generate "foo.N.pdf" and
# "foo.N.txt" output (see "man-pdf" & "man-txt" targets)
MAN = man
MAN_FLAGS =

# The "ps2pdf" command is part of GhostSript (gs) distro.
# It is just a wrapper script around gs that does this:
#
#   gs -dCompatibilityLevel=1.2 -q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite \
#     "-sOutputFile=$outfile" -dCompatibilityLevel=1.2 -c .setpdfwrite -f "$infile"
#
# Where, for example: $outfile = foo.pdf and $infile = foo.1
PS2PDF = ps2pdf
PS2PDF_FLAGS =

# The "col" command is needed for stripping backspaces and
# underscores from man(1) output to get pure plain-text
COL = col
COL_FLAGS =

# The "expand" command is needed for expanding tabs from files
# generated from "man foo.1 | col -b" output
# output to get pure plain-text
EXPAND = expand
EXPAND_FLAGS =

# value of DOCBOOK_XSL should either be the canonical
# (docbook.sourceforge.net) URL for the DocBook Project XSL
# stylesheets OR it can be a local system path
DOCBOOK_XSL = http://docbook.sourceforge.net/release/xsl/current

# -----------------------------------------------------------------
#       names of some DIRECTORIES and FILES we need
# -----------------------------------------------------------------
# We create a tmp directory once per make invocation; it's needed
# for holding a temporary copy of the custom DBLaTeX stylesheet
# (because dblatex currently can't read a stylesheet from stdin)
TMP ?= /tmp
TMPNUM := $(shell echo $$$$)
DOCBOOK_TMP := $(TMP)/docbook-make-$(TMPNUM)

# MAN_MANIFEST_EXT is file extension added to individual manifest
# files
MAN_MANIFEST_EXT = manifest_man

# HTML_MANIFEST_EXT is file extension added to HTML manifest files
HTML_MANIFEST_EXT = manifest_html

# BASEDIR_SUFFIX is a what you need to set if you want a suffix
# added to the end of each "base.dir" we create while generating
# chunked HTML output
#BASEDIR_SUFFIX = -html
#BASEDIR_SUFFIX = _html
BASEDIR_SUFFIX = 

#
# Workaround the fact that we don't calculate dependencies.
#

BROKEN_DEPENDENCIES = package/changelog/index.in $(shell find . -name "*.xml") package/changelog/index.xml make-changelog Makefile


# -----------------------------------------------------------------
#                        assorted OPTIONS
# -----------------------------------------------------------------
# HTML_STYLESHEET -> $html.stylesheet stylesheet param
# http://docbook.sourceforge.net/snapshots/xsl/doc/html/html.stylesheet.html
HTML_STYLESHEET = style.css
# HTML_IMAGES -> $admon.graphics.path
# http://docbook.sourceforge.net/snapshots/xsl/doc/html/admon.graphics.path.html
HTML_IMAGES = images/
# HTML_IMAGES_EXT -> $admon.graphics.extension
# http://docbook.sourceforge.net/snapshots/xsl/doc/html/admon.graphics.extension.html
HTML_IMAGES_EXT = .png

# use these to set params on the command-line
# format is, e.g., HTML_PARAMS="--stringparam variablelist.as.table 1..."
FO_PARAMS =
HTML_PARAMS =
MAN_PARAMS =

# DBX_PARAMS is for dblatex(1); format uses "-p":
# DBX_PARAMS="-p doc.publisher.show 1 -p term.breakline 1...
DBX_PARAMS=

# What file extension do you use for DocBook source files?
DOCBOOK_FILE_EXTENSION = .docbook
SOURCE_FILE_EXTENSION = .xml

# -----------------------------------------------------------------
#       make(1) functions for building file lists
# -----------------------------------------------------------------
#
#    the values of the following are used for determing what needs
#    to be built and/or cleaned up

SOURCE_FILES_DBK = $(wildcard *$(SOURCE_FILE_EXTENSION))

FILES_FO   = $(foreach base,$(basename $(SOURCE_FILES_DBK)),$(base).fo)
FILES_TXT  = $(foreach base,$(basename $(SOURCE_FILES_DBK)),$(base).txt)
FILES_PDF  = $(foreach base,$(basename $(SOURCE_FILES_DBK)),$(base).pdf)
FILES_LOG  = $(foreach base,$(basename $(SOURCE_FILES_DBK)),$(base).log)
FILES_OUT  = $(foreach base,$(basename $(SOURCE_FILES_DBK)),$(base).out)
FILES_AUX  = $(foreach base,$(basename $(SOURCE_FILES_DBK)),$(base).aux)
FILES_HTML = $(foreach base,$(basename $(SOURCE_FILES_DBK)),$(base).html)
FILES_INFO = $(foreach base,$(basename $(SOURCE_FILES_DBK)),$(base).info)
DIRS_CHUNK = $(foreach base,$(basename $(SOURCE_FILES_DBK)),$(base)$(BASEDIR_SUFFIX))
LISTS_HTML = $(foreach base,$(basename $(SOURCE_FILES_DBK)),$(base).$(HTML_MANIFEST_EXT))
LISTS_MAN  = $(foreach base,$(basename $(SOURCE_FILES_DBK)),$(base).$(MAN_MANIFEST_EXT))
FILES_CHNK = $(shell for manifest in $(LISTS_HTML); do if [ -f "$$manifest" ]; then cat $$manifest; fi done)
FILES_MAN  = $(shell for manifest in $(LISTS_MAN);  do if [ -f "$$manifest" ]; then cat $$manifest; fi done)
FILES_MANP = $(foreach base,$(FILES_MAN),$(base).pdf)
FILES_MANT = $(foreach base,$(FILES_MAN),$(base).txt)
DIRS_MAN   = $(shell for file in $(FILES_MAN); do dirname $$file; done | uniq)

# -----------------------------------------------------------------
#   ** stylesheet for testing whether a file has a refentry
# -----------------------------------------------------------------
REFENTRY_CHECK := <xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" \
                version="1.0" \
                xmlns:db="http://docbook.org/ns/docbook" \
                > \
  <xsl:output method="text"/> \
  <xsl:template match="/"> \
    <xsl:if test="//refentry|//db:refentry"> \
      <xsl:text>true</xsl:text> \
    </xsl:if> \
  </xsl:template> \
</xsl:stylesheet>

# -----------------------------------------------------------------
#             ** Stylesheet Customization Layers ** 
# -----------------------------------------------------------------
#
# for DBLaTeX
DBX_CUSTOM := <xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" \
                 version="1.0"> \
  <xsl:param name="co.linkends.show">0</xsl:param> \
  <xsl:param name="callout.markup.circled">1</xsl:param> \
  <xsl:param name="callout.linkends.hot">0</xsl:param> \
  <xsl:param name="doc.publisher.show">1</xsl:param> \
  <xsl:param name="term.breakline">1</xsl:param> \
  <xsl:param name="doc.alignment">left</xsl:param> \
  <xsl:param name="newtbl.use">1</xsl:param> \
  <xsl:param name="latex.hyperparam"/> \
  <xsl:param name="latex.style">docbook</xsl:param> \
  <xsl:param name="latex.biblio.output">all</xsl:param> \
  <xsl:param name="latex.bibfiles">""</xsl:param> \
  <xsl:param name="latex.bibwidelabel">WIDELABEL</xsl:param> \
  <xsl:param name="latex.output.revhistory">0</xsl:param> \
  <xsl:param name="latex.figure.position">[htbp]</xsl:param> \
  <xsl:param name="latex.figure.boxed">0</xsl:param> \
  <xsl:param name="latex.babel.use">1</xsl:param> \
  <xsl:param name="latex.babel.language"></xsl:param> \
  <xsl:param name="latex.class.options">twoside</xsl:param> \
  <xsl:param name="biblioentry.item.separator">, </xsl:param> \
  <xsl:param name="refentry.xref.manvolnum">1</xsl:param> \
  <xsl:param name="refsynopsis.title">Synopsis</xsl:param> \
  <xsl:param name="refnamediv.title"></xsl:param> \
  <xsl:param name="funcsynopsis.style">ansi</xsl:param> \
  <xsl:param name="funcsynopsis.decoration">1</xsl:param> \
  <xsl:param name="function.parens">0</xsl:param> \
  <xsl:param name="classsynopsis.default.language">java</xsl:param> \
  <xsl:param name="show.comments">0</xsl:param> \
</xsl:stylesheet>

# for FO output

#
# NOTE: The following changes may be required to the stock docbook stylesheets
# for double-sided output:
#
# In /usr/share/xml/docbook/stylesheet/nwalsh/fo:
#
# 	o Add '<xsl:attribute name="margin-left">0pc</xsl:attribute>'
#	  to the template 'footer.table'.  This fixes number positioning in
#	  double-sided output.
#	o In the test for generating 'section.head.marker', change
#	  the test '$double-sided != 0' to read '$double-sided > 1'.
#	  This disables section-name headings in favor of chapter-name
#	  headings.
#
#	--dhilvert@auricle.dyndns.org, 12-Oct-2006
#
FO_CUSTOM := <xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" \
                xmlns:fo="http://www.w3.org/1999/XSL/Format" \
                version="1.0"> \
  <xsl:import href="$(DOCBOOK_XSL)/fo/docbook.xsl"/> \
  <xsl:param name="$(PDF_MAKER).extensions">1</xsl:param> \
  <xsl:param name="paper.type">Custom</xsl:param> \
  <xsl:param name="commented.page.height">9.25in</xsl:param> \
  <xsl:param name="commented.page.width">7.5in</xsl:param> \
  <xsl:param name="page.height">PAPER_HEIGHT</xsl:param> \
  <xsl:param name="page.width">PAPER_WIDTH</xsl:param> \
  <xsl:param name="commented.page.margin.inner">3in</xsl:param> \
  <xsl:param name="commented.page.margin.outer">3in</xsl:param> \
  <xsl:param name="chapter.autolabel">1</xsl:param> \
  <xsl:param name="section.autolabel">1</xsl:param> \
  <xsl:param name="toc.section.depth">2</xsl:param> \
  <xsl:param name="commented.toc.max.depth">2</xsl:param> \
  <xsl:param name="double.sided">DOUBLE_SIDED</xsl:param> \
  <xsl:param name="draft.watermark.image"></xsl:param> \
  <xsl:param name="hyphenation">false</xsl:param> \
  <xsl:param name="hyphenate.verbatim">1</xsl:param> \
  <xsl:param name="alignment">left</xsl:param> \
  <xsl:param name="refentry.generate.name">1</xsl:param> \
  <xsl:param name="refentry.generate.title">0</xsl:param> \
  <xsl:param name="refentry.pagebreak">1</xsl:param> \
  <xsl:param name="shade.verbatim">1</xsl:param> \
  <xsl:param name="variablelist.as.blocks">1</xsl:param> \
  <xsl:param name="ulink.show">1</xsl:param> \
  <xsl:param name="ulink.footnotes">1</xsl:param> \
  <xsl:param name="index.on.type">1</xsl:param> \
  <xsl:attribute-set name="xref.properties"> \
    <xsl:attribute name="color">XREF_COLOR</xsl:attribute> \
  </xsl:attribute-set> \
  <xsl:template match="section[@tocexclude = 1]"  mode="toc" /> \
  <xsl:template name="commented.page.number.format"> \
  <xsl:param name="element" select="local-name(.)"/> \
  <xsl:choose> \
    <xsl:when test="$element = 'toc'">i</xsl:when> \
    <xsl:when test="$element = 'preface'">i</xsl:when> \
    <xsl:when test="$element = 'dedication'">i</xsl:when> \
    <xsl:when test="$element = 'book'">i</xsl:when> \
    <xsl:when test="$element = 'set'">i</xsl:when> \
    <xsl:otherwise>1</xsl:otherwise> \
  </xsl:choose> \
  </xsl:template> \
  <xsl:template match="*" mode="toc.for.set"> \
    <xsl:call-template name="set.toc"/> \
  </xsl:template> \
  <xsl:attribute-set name="shade.verbatim.style"> \
    <xsl:attribute name="background-color">\#E0E0E0</xsl:attribute> \
    <xsl:attribute name="padding-left">4pt</xsl:attribute> \
    <xsl:attribute name="padding-right">4pt</xsl:attribute> \
    <xsl:attribute name="padding-top">4pt</xsl:attribute> \
    <xsl:attribute name="padding-bottom">4pt</xsl:attribute> \
  </xsl:attribute-set> \
  <xsl:attribute-set name="section.title.level1.properties"> \
    <xsl:attribute name="font-size"> \
      <xsl:value-of select="$$body.font.master * 1.5"></xsl:value-of> \
      <xsl:text>pt</xsl:text> \
    </xsl:attribute> \
  </xsl:attribute-set> \
  <xsl:attribute-set name="section.title.level2.properties"> \
    <xsl:attribute name="font-size"> \
      <xsl:value-of select="$$body.font.master * 1.3"></xsl:value-of> \
      <xsl:text>pt</xsl:text> \
    </xsl:attribute> \
  </xsl:attribute-set> \
  <xsl:attribute-set name="section.title.level3.properties"> \
    <xsl:attribute name="font-size"> \
      <xsl:value-of select="$$body.font.master * 1.1"></xsl:value-of> \
      <xsl:text>pt</xsl:text> \
    </xsl:attribute> \
  </xsl:attribute-set> \
  <xsl:attribute-set name="section.title.level4.properties"> \
    <xsl:attribute name="font-size"> \
      <xsl:value-of select="$$body.font.master"></xsl:value-of> \
      <xsl:text>pt</xsl:text> \
    </xsl:attribute> \
  </xsl:attribute-set> \
  <xsl:attribute-set name="component.title.properties"> \
    <xsl:attribute name="font-size"> \
      <xsl:value-of select="$$body.font.master * 1.5"></xsl:value-of> \
      <xsl:text>pt</xsl:text> \
    </xsl:attribute> \
  </xsl:attribute-set> \
  <xsl:attribute-set name="monospace.verbatim.properties"> \
    <xsl:attribute name="wrap-option">wrap</xsl:attribute> \
    <xsl:attribute name="hyphenation-character">\</xsl:attribute> \
    <xsl:attribute name="font-size"> \
      <xsl:value-of select="$$body.font.master * 0.8"></xsl:value-of> \
      <xsl:text>pt</xsl:text> \
    </xsl:attribute> \
  </xsl:attribute-set> \
</xsl:stylesheet>

FO_LETTER_COLOR := $(subst PAPER_WIDTH, 8.5in, \
                   $(subst PAPER_HEIGHT, 11in, \
		   $(subst XREF_COLOR,blue, \
		   $(subst DOUBLE_SIDED, 0,  \
		   	$(FO_CUSTOM)))))

FO_LETTER_MONO  := $(subst PAPER_WIDTH, 8.5in, \
                   $(subst PAPER_HEIGHT, 11in, \
		   $(subst XREF_COLOR,black, \
		   $(subst DOUBLE_SIDED, 0,  \
		   	$(FO_CUSTOM)))))

FO_SMALL_COLOR  := $(subst PAPER_WIDTH, 7.5in, \
                   $(subst PAPER_HEIGHT, 9.25in, \
		   $(subst XREF_COLOR,blue, \
		   $(subst DOUBLE_SIDED, 1,  \
		   	$(FO_CUSTOM)))))

FO_SMALL_MONO   := $(subst PAPER_WIDTH, 7.5in, \
                   $(subst PAPER_HEIGHT, 9.25in, \
		   $(subst XREF_COLOR,black, \
		   $(subst DOUBLE_SIDED, 1,  \
		   	$(FO_CUSTOM)))))

# for single-file (X)HTML outpout
HTML_CUSTOM := <xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" \
                version="1.0"> \
  <xsl:import href="$(DOCBOOK_XSL)/$(HTML_OR_XHTML)/docbook.xsl"/> \
  <xsl:param name="gentext.custom">local.l10n.xml</xsl:param> \
  <xsl:param name="local.l10n.xml" select="document($$gentext.custom)"/> \
  <xsl:param name="refentry.generate.name">1</xsl:param> \
  <xsl:param name="refentry.generate.title">0</xsl:param> \
  <xsl:param name="root.filename"></xsl:param> \
  <xsl:param name="variablelist.as.table">0</xsl:param> \
  <xsl:param name="html.stylesheet">$(HTML_STYLESHEET)</xsl:param> \
  <xsl:param name="admon.graphics">0</xsl:param> \
  <xsl:param name="index.on.type">1</xsl:param> \
  <xsl:param name="chapter.autolabel">1</xsl:param> \
  <xsl:param name="section.autolabel">1</xsl:param> \
  <xsl:param name="toc.section.depth">2</xsl:param> \
  <xsl:param name="commented.toc.max.depth">2</xsl:param> \
  <xsl:template match="section[@tocexclude = 1]"  mode="toc" /> \
  <xsl:param name="generate.toc"> \
appendix  toc,title \
article/appendix  nop \
article   toc, title \
book      toc,title,figure,table,example,equation \
chapter   toc,title \
part      toc,title \
preface   toc,title \
qandadiv  toc \
qandaset  toc \
reference toc,title \
sect1     toc \
sect2     toc \
sect3     toc \
sect4     toc \
sect5     toc \
section   toc \
set       toc,title \
</xsl:param> \
</xsl:stylesheet>

# for chunked (X)HTML output
CHNK_CUSTOM := <xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" \
                version="1.0"> \
  <xsl:import href="$(DOCBOOK_XSL)/$(HTML_OR_XHTML)/chunk.xsl"/> \
  <xsl:param name="refentry.generate.name">0</xsl:param> \
  <xsl:param name="refentry.generate.title">1</xsl:param> \
  <xsl:param name="variablelist.as.table">0</xsl:param> \
  <xsl:param name="html.stylesheet">$(HTML_STYLESHEET)</xsl:param> \
  <xsl:param name="admon.graphics">1</xsl:param> \
  <xsl:param name="admon.graphics.path">$(HTML_IMAGES)</xsl:param> \
  <xsl:param name="admon.graphics.extension">$(HTML_IMAGES_EXT)</xsl:param> \
  <xsl:param name="root.filename"></xsl:param> \
  <xsl:param name="generate.manifest">1</xsl:param> \
  <xsl:param name="use.id.as.filename">1</xsl:param> \
  <xsl:param name="chunker.output.indent">yes</xsl:param> \
  <xsl:param name="index.on.type">1</xsl:param> \
  <xsl:param name="chapter.autolabel">1</xsl:param> \
  <xsl:param name="section.autolabel">1</xsl:param> \
  <xsl:param name="toc.section.depth">2</xsl:param> \
  <xsl:param name="commented.toc.max.depth">2</xsl:param> \
  <xsl:template match="section[@tocexclude = 1]"  mode="toc" /> \
  <xsl:param name="generate.toc"> \
appendix  toc,title \
article/appendix  nop \
article   toc,title \
book      toc,title,figure,table,example,equation \
chapter   toc,title \
part      toc,title \
preface   toc,title \
qandadiv  toc \
qandaset  toc \
reference toc,title \
sect1     toc \
sect2     toc \
sect3     toc \
sect4     toc \
sect5     toc \
section   toc \
set       toc,title \
</xsl:param> \
</xsl:stylesheet>

# for man-page output
MAN_CUSTOM := <xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" \
                version="1.0"> \
  <xsl:import href="$(DOCBOOK_XSL)/manpages/docbook.xsl"/> \
  <xsl:param name="man.output.manifest.enabled" select="1"/> \
  <xsl:param name="man.output.in.separate.dir" select="1"/> \
  <xsl:param name="man.output.base.dir">man/</xsl:param> \
  <xsl:param name="man.output.subdirs.enabled" select="1"/> \
</xsl:stylesheet>

#
# ALE documentation targets
#

ALE_SUBDIRS := user technical
ALE_TYPES := letter.pdf txt info
ALE_PACKAGE := TODO ChangeLog NEWS README REPORTING-BUGS
ALE_MANUALS := $(foreach subdir, $(ALE_SUBDIRS), $(subdir)-manual)
ALE_MANUAL_TARGETS := $(foreach manual, $(ALE_MANUALS), $(foreach type, $(ALE_TYPES), $(manual).$(type)))
ALE_PACKAGE_TARGETS := $(foreach package, $(ALE_PACKAGE), ../$(package))
ALE_TARGETS := $(ALE_MANUAL_TARGETS) $(ALE_PACKAGE_TARGETS)


# -----------------------------------------------------------------
#                  ** TARGETS START HERE ** 
# -----------------------------------------------------------------
#  prevents make from deleting, e.g. foo.1, after making foo.1.pdf
#  and foo.1.txt from it.
.PRECIOUS: %.1 %.2 %.3 %.4 %.5 %.6 %.7 %.8 %.9

# docbook: index.docbook $(DOCBOOK_OUTPUT_FORMATS) ../TODO ../ChangeLog ../NEWS ../README ../REPORTING-BUGS
docbook: $(ALE_TARGETS) index.docbook chunk
	rm -rf html
	mv index html
	mkdir -p info; mv *.info info
	rm index.docbook

%-manual.docbook: %/index.xml
	$(XSLT) $(XSLT_FLAGS) ale-doc.xsl $< > $@
	
manual-set.docbook: index.xml
	$(XSLT) $(XSLT_FLAGS) ale-doc.xsl $< > $@

../TODO: package/todo/index.txt
	mv package/todo/index.txt ../TODO
	
../ChangeLog: package/changelog/index.txt
	mv package/changelog/index.txt ../ChangeLog
	
../NEWS: package/news/index.txt
	mv package/news/index.txt ../NEWS
	
../README: package/readme/index.txt
	mv package/readme/index.txt ../README
	
man: $(LISTS_MAN)

# we can generate PDFs from man pages
man-pdf: man
	$(MAKE) $(FILES_MANP)

# we can generate plain text from man pages
man-txt: man
	$(MAKE) $(FILES_MANT)

html: $(FILES_HTML)

chunk: $(LISTS_HTML)

txt: $(FILES_TXT)

pdf: $(FILES_PDF)

info: $(FILES_INFO)

# use the "debug" target to echo variables, etc., to
# test/troubleshoot changes you make to this makefile
debug:
	@echo $(DIRS_MAN)

# -----------------------------------------------------------------
#     preprocess xml to docbook.
# -----------------------------------------------------------------
%$(DOCBOOK_FILE_EXTENSION): %.xml $(BROKEN_DEPENDENCIES) ale-doc.xsl
	$(XSLT) $(XSLT_FLAGS) ale-doc.xsl $< > $@

package/changelog/index.xml: package/changelog/index.in ./make-changelog
	./make-changelog < package/changelog/index.in > package/changelog/index.xml

# -----------------------------------------------------------------
#     pattern rule for making (X)HTML and plain-text output
# -----------------------------------------------------------------
%.html: %$(DOCBOOK_FILE_EXTENSION) $(BROKEN_DEPENDENCIES)
	@echo '$(HTML_CUSTOM)' | $(XSLT) $(XSLT_FLAGS) $(HTML_PARAMS) - $< > $@

%.txt: %.html
	$(TXT_MAKER) $(TXT_MAKER_FLAGS) ./$< \
	  | sed "s/^\(\s\+[0-9]\+\. \)file:\/\/.\+$$/\\1(local)/g" \
	  | egrep -v '^	file:///.+$$' \
	  > $@
# if DOCBOOK_OUTPUT_FORMATS does not contain "html", then we need
# to remove the "intermediate" HTML files we used for generating
# plain-text output
ifeq ($(findstring html,$(DOCBOOK_OUTPUT_FORMATS)),)
	$(RM) $<
endif

# -----------------------------------------------------------------
#     pattern rule for making chunked (X)HTML pages
# -----------------------------------------------------------------
%.$(HTML_MANIFEST_EXT): %$(DOCBOOK_FILE_EXTENSION) $(BROKEN_DEPENDENCIES)
	@echo '$(CHNK_CUSTOM)' | $(XSLT) $(XSLT_FLAGS) $(HTML_PARAMS) \
	--stringparam manifest $@  \
	--stringparam base.dir $(basename $@)$(BASEDIR_SUFFIX)/  \
	- $<

# -----------------------------------------------------------------
#     pattern rules for making FO and PDF stuff
# -----------------------------------------------------------------
%.letter.fo: %$(DOCBOOK_FILE_EXTENSION) $(BROKEN_DEPENDENCIES)
	@echo '$(FO_LETTER_COLOR)' \
	  | $(XSLT) $(XSLT_FLAGS) $(FO_PARAMS) - $< > $@

%.small.fo: %$(DOCBOOK_FILE_EXTENSION) $(BROKEN_DEPENDENCIES)
	@echo '$(FO_SMALL_COLOR)' \
	  | $(XSLT) $(XSLT_FLAGS) $(FO_PARAMS) - $< > $@

%.letter-mono.fo: %$(DOCBOOK_FILE_EXTENSION) $(BROKEN_DEPENDENCIES)
	@echo '$(FO_LETTER_MONO)' \
	  | $(XSLT) $(XSLT_FLAGS) $(FO_PARAMS) - $< > $@

%.small-mono.fo: %$(DOCBOOK_FILE_EXTENSION) $(BROKEN_DEPENDENCIES)
	@echo '$(FO_SMALL_MONO)' \
	  | $(XSLT) $(XSLT_FLAGS) $(FO_PARAMS) - $< > $@

ifeq ($(PDF_MAKER),dblatex)
%.pdf: %$(DOCBOOK_FILE_EXTENSION)
	mkdir -p $(DOCBOOK_TMP)
	echo '$(DBX_CUSTOM)' > $(DOCBOOK_TMP)/dblatex.xsl
	-$(DBLATEX) $(DBLATEX_FLAGS)\
	  -p $(DOCBOOK_TMP)/dblatex.xsl \
	  -o $@ \
	  $<
	$(RM) -r $(DOCBOOK_TMP)
endif

%.pdf: %.fo
ifeq ($(PDF_MAKER),)
	$(error No PDF_MAKER specified. Cannot make pdf)
else
ifeq ($(PDF_MAKER),xep)
	$(XEP) $(XEP_FLAGS) $< $@
else
ifeq ($(PDF_MAKER),fop)
	$(FOP) $(FOP_FLAGS) $< $@
else
ifeq ($(PDF_MAKER),xmlroff)
	$(XMLROFF) $(XMLROFF_FLAGS) $< -o $@
else
ifeq ($(PDF_MAKER),passivetex)
	$(PDFTEX) $(PDFTEX_FLAGS) &$(PDFXMLTEX) $<
	@if [ `egrep Rerun $(basename $@).log | wc -l` -gt 0 ]; then \
		$(PDFTEX) $(PDFTEX_FLAGS) &$(PDFXMLTEX) $< ; \
	fi
	@if [ `egrep Rerun $(basename $@).log | wc -l` -gt 0 ]; then \
		$(PDFTEX) $(PDFTEX_FLAGS) &$(PDFXMLTEX) $< ; \
	fi
	$(RM) $(basename $@).log
	$(RM) $(basename $@).aux
	$(RM) $(basename $@).out
else
	$(error I do not know how to make a PDF using "$(PDF_MAKER)")
endif
endif
endif
endif
endif

# -----------------------------------------------------------------
#     pattern rules for making TeXinfo stuff
# -----------------------------------------------------------------
%.txml: %$(DOCBOOK_FILE_EXTENSION)
	$(DB2X_XSLTPROC) $(DB2X_XSLTPROC_FLAGS) -s texi -o $@ $<

%.texi-list: %.txml
	$(DB2X_TEXIXML) $(DB2X_TEXIXML_FLAGS) $< > $@

# the following is actually a built-in rule, but it's redefined
# here just for the sake of clarity
%.info: %.texi-list
	/usr/bin/perl -w make-infos $(MAKEINFO) < $<
	-@ # This doesn't work in general.
	-@ # $(MAKEINFO) $(MAKEINFO_FLAGS) $< -o $@

# -----------------------------------------------------------------
#     pattern rule for making man pages
# -----------------------------------------------------------------
%.$(MAN_MANIFEST_EXT): %$(DOCBOOK_FILE_EXTENSION)
	@if [ "$(strip $(shell echo '$(REFENTRY_CHECK)' | $(XSLT) $(XSLT_FLAGS) - $<))" != "true" ]; then \
	  touch $@; \
	else \
	  echo '$(MAN_CUSTOM)' \
	  | $(XSLT) $(XSLT_FLAGS) $(MAN_PARAMS) \
	    --stringparam man.output.manifest.filename $@ \
	    - $<; \
	fi

# -----------------------------------------------------------------
#  pattern rule for enabling direct "make foo.1" to work
# -----------------------------------------------------------------

%.1 %.2 %.3 %.4 %.5 %.6 %.7: %.xml
	$(MAKE) $(basename $<).$(MAN_MANIFEST_EXT)

# -----------------------------------------------------------------
#  pattern rules for making Postscript/PDF output from man pages
# -----------------------------------------------------------------
%.1.ps: %.1
	$(MAN) -l $(MAN_FLAGS) -Tps $< > $@

%.2.ps: %.2
	$(MAN) -l $(MAN_FLAGS) -Tps $< > $@

%.3.ps: %.3
	$(MAN) -l $(MAN_FLAGS) -Tps $< > $@

%.4.ps: %.4
	$(MAN) -l $(MAN_FLAGS) -Tps $< > $@

%.5.ps: %.5
	$(MAN) -l $(MAN_FLAGS) -Tps $< > $@

%.6.ps: %.6
	$(MAN) -l $(MAN_FLAGS) -Tps $< > $@

%.7.ps: %.7
	$(MAN) -l $(MAN_FLAGS) -Tps $< > $@

%.8.ps: %.8
	$(MAN) -l $(MAN_FLAGS) -Tps $< > $@

%.9.ps: %.9
	$(MAN) -l $(MAN_FLAGS) -Tps $< > $@

# -----------------------------------------------------------------
#  pattern rule for converting Postscript to PDF
# -----------------------------------------------------------------
%.pdf: %.ps
	$(PS2PDF) $(PS2PDF_FLAGS) $<

# -----------------------------------------------------------------
#  pattern rules for making plain-text output from man pages
# -----------------------------------------------------------------
%.1.txt: %.1
	$(MAN) -l $(MAN_FLAGS) -Tascii $< | $(COL) -b $(COL_FLAGS) | $(EXPAND) $(EXPAND_FLAGS) > $@

%.2.txt: %.2
	$(MAN) -l $(MAN_FLAGS) -Tascii $< | $(COL) -b $(COL_FLAGS) | $(EXPAND) $(EXPAND_FLAGS) > $@

%.3.txt: %.3
	$(MAN) -l $(MAN_FLAGS) -Tascii $< | $(COL) -b $(COL_FLAGS) | $(EXPAND) $(EXPAND_FLAGS) > $@

%.4.txt: %.4
	$(MAN) -l $(MAN_FLAGS) -Tascii $< | $(COL) -b $(COL_FLAGS) | $(EXPAND) $(EXPAND_FLAGS) > $@

%.5.txt: %.5
	$(MAN) -l $(MAN_FLAGS) -Tascii $< | $(COL) -b $(COL_FLAGS) | $(EXPAND) $(EXPAND_FLAGS) > $@

%.6.txt: %.6
	$(MAN) -l $(MAN_FLAGS) -Tascii $< | $(COL) -b $(COL_FLAGS) | $(EXPAND) $(EXPAND_FLAGS) > $@

%.7.txt: %.7
	$(MAN) -l $(MAN_FLAGS) -Tascii $< | $(COL) -b $(COL_FLAGS) | $(EXPAND) $(EXPAND_FLAGS) > $@

%.8.txt: %.8
	$(MAN) -l $(MAN_FLAGS) -Tascii $< | $(COL) -b $(COL_FLAGS) | $(EXPAND) $(EXPAND_FLAGS) > $@

%.9.txt: %.9
	$(MAN) -l $(MAN_FLAGS) -Tascii $< | $(COL) -b $(COL_FLAGS) | $(EXPAND) $(EXPAND_FLAGS) > $@

# -----------------------------------------------------------------
#   target(s) for cleaning up the mess
# -----------------------------------------------------------------
clean:
	find . -name "*.info" | xargs rm -f
	find . -name "*.texi" | xargs rm -f
	find . -name "*.texi-list" | xargs rm -f
	find . -name "*.pdf" | xargs rm -f
	find . -name "*.fo" | xargs rm -f
	find . -name "*.docbook" | xargs rm -f
	find . -name "*.txt" | xargs rm -f
	rm -f package/changelog/index.xml
	rm -rf index
	rm -rf html
ifneq ($(FILES_TXT),)
	$(RM) $(FILES_TXT)
endif
ifneq ($(FILES_PDF),)
	$(RM) $(FILES_PDF)
endif
ifneq ($(FILES_FO),)
	$(RM) $(FILES_FO)
endif
ifneq ($(FILES_LOG),)
	$(RM) $(FILES_LOG)
endif
ifneq ($(FILES_OUT),)
	$(RM) $(FILES_OUT)
endif
ifneq ($(FILES_AUX),)
	$(RM) $(FILES_AUX)
endif
ifneq ($(FILES_HTML),)
	$(RM) $(FILES_HTML)
endif
ifneq ($(FILES_CHNK),)
	$(RM) $(FILES_CHNK)
endif
ifneq ($(FILES_MAN),)
	$(RM) $(FILES_MAN)
endif
ifneq ($(FILES_MANT),)
	$(RM) $(FILES_MANT)
endif
ifneq ($(FILES_MANP),)
	$(RM) $(FILES_MANP)
endif
ifneq ($(FILES_INFO),)
	$(RM) $(FILES_INFO)
endif
ifneq ($(DIRS_CHUNK),)
	for dir in $(DIRS_CHUNK); do \
	  if [ -d "$$dir" ]; then \
	    $(RMDIR) $(RMDIR_FLAGS) $$dir; \
	  fi \
	done
endif
ifneq ($(DIRS_MAN),)
ifneq ($(DIRS_MAN),.)
	for dir in $(DIRS_MAN); do \
	  if [ -d "$$dir" ]; then \
	    $(RMDIR) $(RMDIR_FLAGS) $$dir; \
	  fi \
	done
endif
endif
ifneq ($(LISTS_MAN),)
	$(RM) $(LISTS_MAN)
endif
ifneq ($(LISTS_HTML),)
	$(RM) $(LISTS_HTML)
endif
