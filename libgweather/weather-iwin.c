/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Weather server functions (IWIN)
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>
#include <glib/gi18n-lib.h>

#include <libgweather/weather.h>
#include "weather-priv.h"

#define IWIN_RE_STR "([A-Z][A-Z]Z(([0-9]{3}>[0-9]{3}-)|([0-9]{3}-))+)+([0-9]{6}-)?"

/**
 * Unused. Are these functions useful?
 */
#if 0

static regex_t iwin_re;

static void iwin_init_re (void)
{
    static gboolean initialized = FALSE;
    if (initialized)
        return;
    initialized = TRUE;

    regcomp(&iwin_re, IWIN_RE_STR, REG_EXTENDED);
}

#define CONST_ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

static gboolean iwin_range_match (gchar *range, WeatherLocation *loc)
{
    gchar *zonep;
    gchar *endp;
    gchar zone_state[4], zone_num_str[4];
    gint zone_num;

    endp = range + strcspn(range, " \t\r\n") - 1;
    if (strspn(endp - 6, CONST_DIGITS) == 6) {
        --endp;
        while (*endp != '-')
            --endp;
    }
    g_assert(range <= endp);

    strncpy(zone_state, loc->zone, 3);
    zone_state[3] = 0;
    strncpy(zone_num_str, loc->zone + 3, 3);
    zone_num_str[3] = 0;
    zone_num = atoi(zone_num_str);

    zonep = range;
    while (zonep < endp) {
        gchar from_str[4], to_str[4];
        gint from, to;

        if (strncmp(zonep, zone_state, 3) != 0) {
            zonep += 3;
            zonep += strcspn(zonep, CONST_ALPHABET "\n");
            continue;
        }

        zonep += 3;

        do {
            strncpy(from_str, zonep, 3);
            from_str[3] = 0;
            from = atoi(from_str);

            zonep += 3;

            if (*zonep == '-') {
                ++zonep;
                to = from;
            } else if (*zonep == '>') {
                ++zonep;
                strncpy(to_str, zonep, 3);
                to_str[3] = 0;
                to = atoi(to_str);
                zonep += 4;
            } else {
                g_assert_not_reached();
                to = 0;  /* Hush compiler warning */
            }

            if ((from <= zone_num) && (zone_num <= to))
                return TRUE;

        } while (!isupper(*zonep) && (zonep < endp));
    }

    return FALSE;
}
#endif

/**
 *  Human's don't deal well with .MONDAY...SUNNY AND BLAH BLAH.TUESDAY...THEN THIS AND THAT.WEDNESDAY...RAINY BLAH BLAH.
 *  This function makes it easier to read.
 */
static gchar* formatWeatherMsg (gchar* forecast) {

    gchar* ptr = forecast;
    gchar* startLine = NULL;

    while (0 != *ptr) {
        if (ptr[0] == '\n' && ptr[1] == '.') {
            if (NULL == startLine) {
                memmove(forecast, ptr, strlen(ptr) + 1);
                ptr[0] = ' ';
                ptr = forecast;
            }
            ptr[1] = '\n';
            ptr += 2;
            startLine = ptr;

        } else if (ptr[0] == '.' && ptr[1] == '.' && ptr[2] == '.' && NULL != startLine) {
            memmove(startLine + 2, startLine, (ptr - startLine) * sizeof(gchar));
            startLine[0] = ' ';
            startLine[1] = '\n';
            ptr[2] = '\n';

            ptr += 3;

        } else if (ptr[0] == '$' && ptr[1] == '$') {
            ptr[0] = ptr[1] = ' ';

        } else {
            ptr++;
        }
    }

    return forecast;
}


static void iwin_finish_read(GnomeVFSAsyncHandle *handle, GnomeVFSResult result, 
			     gpointer buffer, GnomeVFSFileSize requested, 
			     GnomeVFSFileSize body_len, gpointer data)
{
    WeatherInfo *info = (WeatherInfo *)data;
    gchar *body, *temp;
    
    g_return_if_fail(info != NULL);
    g_return_if_fail(handle == info->iwin_handle);

    info->forecast = NULL;
    body = (gchar *)buffer;
    body[body_len] = '\0';

    if (info->iwin_buffer == NULL)
	info->iwin_buffer = g_strdup(body);
    else
    {
	temp = g_strdup(info->iwin_buffer);
	g_free(info->iwin_buffer);
	info->iwin_buffer = g_strdup_printf("%s%s", temp, body);
	g_free(temp);
    }
	
    if (result == GNOME_VFS_ERROR_EOF)
    {
        info->forecast = formatWeatherMsg(g_strdup (info->iwin_buffer));
    }
    else if (result != GNOME_VFS_OK) {
	g_print("%s", gnome_vfs_result_to_string(result));
        g_warning("Failed to get IWIN data.\n");
    } else {
        gnome_vfs_async_read(handle, body, DATA_SIZE - 1, iwin_finish_read, info);
        return;
    }
    
    request_done(info->iwin_handle, info);
    g_free (buffer);
    return;
}

static void iwin_finish_open (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer data)
{
    WeatherInfo *info = (WeatherInfo *)data;
    WeatherLocation *loc;
    gchar *body;

    g_return_if_fail(info != NULL);
    g_return_if_fail(handle == info->iwin_handle);

    body = g_malloc0(DATA_SIZE);

    if (info->iwin_buffer)
    	g_free (info->iwin_buffer);
    info->iwin_buffer = NULL;	
    if (info->forecast)
    g_free (info->forecast);
    info->forecast = NULL;
    loc = info->location;
    if (loc == NULL) {
	    g_warning (_("WeatherInfo missing location"));
	    request_done(info->iwin_handle, info);
	    info->iwin_handle = NULL;
	    requests_done_check(info);
	    g_free (body);
	    return;
    }

    if (result != GNOME_VFS_OK) {
        /* forecast data is not really interesting anyway ;) */
	  g_warning("Failed to get IWIN forecast data.\n"); 
        info->iwin_handle = NULL;
        requests_done_check (info);
        g_free (body);
    } else {
        gnome_vfs_async_read(handle, body, DATA_SIZE - 1, iwin_finish_read, info);
    }
    return;
}

/* Get forecast into newly alloc'ed string */
void iwin_start_open (WeatherInfo *info)
{
    gchar *url, *state, *zone;
    WeatherLocation *loc;

    g_return_if_fail(info != NULL);
    loc = info->location;
    g_return_if_fail(loc != NULL);

    if (loc->zone[0] == '-')
        return;
        
    if (loc->zone[0] == ':')	/* Met Office Region Names */
    {
    	metoffice_start_open (info);
    	return;
    }
    if (loc->zone[0] == '@')    /* Australian BOM forecasts */
    {
    	bom_start_open (info);
    	return;
    }
    
#if 0
    if (info->forecast_type == FORECAST_ZONE)
        url = g_strdup_printf("http://iwin.nws.noaa.gov/iwin/%s/zone.html",
			loc->zone);
    else
        url = g_strdup_printf("http://iwin.nws.noaa.gov/iwin/%s/state.html",
			loc->zone);
#endif
    
    /* The zone for Pittsburgh (for example) is given as PAZ021 in the locations
    ** file (the PA stands for the state pennsylvania). The url used wants the state
    ** as pa, and the zone as lower case paz021.
    */
    zone = g_ascii_strdown (loc->zone, -1);
    state = g_strndup (zone, 2);
    
    url = g_strdup_printf ("http://weather.noaa.gov/pub/data/forecasts/zone/%s/%s.txt",
        		   state, zone); 
    g_free (zone);   
    g_free (state);

    gnome_vfs_async_open(&info->iwin_handle, url, GNOME_VFS_OPEN_READ, 
    			 0, iwin_finish_open, info);
    g_free(url);

}
