NULL =

SUBDIRS = help src

APPLET_LOCATION=$(pkglibdir)/libmultiload-applet.so

gsettings_schemas_in_in = \
	org.gnome.gnome-applets.multiload.gschema.xml.in.in

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

@GSETTINGS_RULES@

EXTRA_DIST =							\
	$(gsettings_schemas_in_in) \
	org.gnome.applets.MultiLoadApplet.panel-applet.in.in	\
	$(service_in_files)					\
	$(ui_DATA)

uidir		= $(pkgdatadir)/ui
ui_DATA		= multiload-applet-menu.xml

appletdir       = $(LIBPANEL_APPLET_DIR)
applet_in_files = org.gnome.applets.MultiLoadApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
            -e "s|\@APPLET_LOCATION\@|$(APPLET_LOCATION)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

CLEANFILES = $(applet_DATA) \
	$(applet_DATA).in \
	$(gsettings_SCHEMAS_in) \
	$(gsettings_SCHEMAS) \
	*.gschema.valid

-include $(top_srcdir)/git.mk
