app_icons = \
	stickynotes-stock-locked.png \
	stickynotes-stock-unlocked.png \
	stickynotes-stock-close.png \
	stickynotes-stock-resize-se.png \
	stickynotes-stock-resize-sw.png

icons = \
	gnome-sticky-notes-applet.16.png \
	gnome-sticky-notes-applet.32.png \
	gnome-sticky-notes-applet.22.png \
	gnome-sticky-notes-applet.24.png \
	gnome-sticky-notes-applet.svg

EXTRA_DIST = \
	$(app_icons) \
	$(icons)

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

uninstall-local:
	for icon in $(icons); do \
		SIZE=`echo $$icon | cut -d. -f2`; \
		FILE=`echo $$icon | cut -d. -f1,3`; \
		if [ "$$SIZE" = "svg" ]; then \
			SIZE="scalable"; \
			FILE="$$FILE.svg"; \
		else \
			SIZE="$${SIZE}x$${SIZE}"; \
		fi; \
		rm -f $(DESTDIR)$(datadir)/icons/hicolor/$$SIZE/apps/$$FILE; \
	done
	for icon in $(app_icons); do \
		rm -f $(DESTDIR)$(pkgdatadir)/icons/hicolor/48x48/apps/$$icon; \
	done
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated. After install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

install-data-local:
	for icon in $(icons); do \
		SIZE=`echo $$icon | cut -d. -f2`; \
		FILE=`echo $$icon | cut -d. -f1,3`; \
		if [ "$$SIZE" = "svg" ]; then \
			SIZE="scalable"; \
			FILE="$$FILE.svg"; \
		else \
			SIZE="$${SIZE}x$${SIZE}"; \
		fi; \
		mkdir -p $(DESTDIR)$(datadir)/icons/hicolor/$$SIZE/apps/; \
		$(INSTALL_DATA) $(srcdir)/$$icon $(DESTDIR)$(datadir)/icons/hicolor/$$SIZE/apps/$$FILE; \
	done
	for icon in $(app_icons); do \
		mkdir -p $(DESTDIR)$(pkgdatadir)/icons/hicolor/48x48/apps/; \
		$(INSTALL_DATA) $(srcdir)/$$icon $(DESTDIR)$(pkgdatadir)/icons/hicolor/48x48/apps/$$icon; \
	done
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated. After install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

-include $(top_srcdir)/git.mk
