/*
    GDP
    TODO: Change malloc to g_malloc, strcpy etc.
    File uses modified code of gnuplot_i available at:
    http://ndevilla.free.fr/gnuplot/
*/

#include "gnuplot_i.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

/* Macros for displaying uint64t */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef WIN32
#include <io.h>
#endif

static inline void gnuplot_plot_atmpfile(gnuplot_ctrl * handle, char const* tmp_filename, char const* title, unsigned char keep_last)
{
    char const *cmd = (keep_last) ? "replot" : "plot";
    title = (title == NULL) ? "(none)" : title;
    gnuplot_cmd(handle, "%s \"%s\" title \"%s\" with %s", cmd, tmp_filename,
                  title, handle->pstyle) ;
    return ;
}

/**
  @brief    Opens up a gnuplot session, ready to receive commands.
  @return   Newly allocated gnuplot control structure.

  This opens up a new gnuplot session, ready for input. The struct
  controlling a gnuplot session should remain opaque and only be
  accessed through the provided functions.

  The session must be closed using gnuplot_close().
 */
gnuplot_ctrl * gnuplot_init(const char * plot_style, const char basechar)
{
    gnuplot_ctrl *  handle ;

#ifndef WIN32
    if (getenv("DISPLAY") == NULL) {
        fprintf(stderr, "cannot find DISPLAY variable: is it set?\n") ;
    }
#endif

    handle = (gnuplot_ctrl*)malloc(sizeof(gnuplot_ctrl)) ;

    /* set style */
    if (strcmp(plot_style, "lines") &&
        strcmp(plot_style, "points") &&
        strcmp(plot_style, "linespoints") &&
        strcmp(plot_style, "impulses") &&
        strcmp(plot_style, "dots") &&
        strcmp(plot_style, "steps") &&
        strcmp(plot_style, "errorbars") &&
        strcmp(plot_style, "boxes") &&
        strcmp(plot_style, "boxerrorbars")) {
        fprintf(stderr, "warning: unknown requested style: using points\n");
        strcpy(handle->pstyle, "points") ;
    } else {
        strcpy(handle->pstyle, plot_style) ;
    }
    strcpy(handle->tmp0name, "gnuplot_tmp_0");
    (handle->tmp0name)[11] = basechar;

    strcpy(handle->tmp1name, "gnuplot_tmp_1");
    (handle->tmp1name)[11] = basechar;
    
    handle->file_no = 0;
    handle->tmp0 = NULL;
    handle->tmp1 = NULL;
    handle->gnucmd = popen("gnuplot  > /dev/null 2>&1 -persist", "w") ;
    if (handle->gnucmd == NULL) {
        fprintf(stderr, "error starting gnuplot, is gnuplot or gnuplot.exe in your path?\n") ;
        free(handle) ;
        return NULL ;
    }

    return handle;
}


/**
  @brief    Closes a gnuplot session previously opened by gnuplot_init()
  @param    handle Gnuplot session control handle.
  @return   void

  Kills the child PID and deletes all opened temporary files.
  It is mandatory to call this function to close the handle, otherwise
  temporary files are not cleaned and child process might survive.

 */
void gnuplot_close(gnuplot_ctrl * handle)
{
    if(handle == NULL)
        return;
    if (pclose(handle->gnucmd) == -1) {
        fprintf(stderr, "problem closing communication to gnuplot\n") ;
        return ;
    }
    remove(handle->tmp0name);
    remove(handle->tmp1name);
    free(handle) ;
    return ;
}


/**
  @brief    Sends a command to an active gnuplot session.
  @param    handle Gnuplot session control handle
  @param    cmd    Command to send, same as a printf statement.

  This sends a string to an active gnuplot session, to be executed.
  There is strictly no way to know if the command has been
  successfully executed or not.
  The command syntax is the same as printf.

  Examples:

  @code
  gnuplot_cmd(g, "plot %d*x", 23.0);
  gnuplot_cmd(g, "plot %.18e * cos(%.18e * x)", 32.0, -3.0);
  @endcode

  Since the communication to the gnuplot process is run through
  a standard Unix pipe, it is only unidirectional. This means that
  it is not possible for this interface to query an error status
  back from gnuplot.
 */
void gnuplot_cmd(gnuplot_ctrl *  handle, char const *  cmd, ...)
{
    va_list ap ;

    va_start(ap, cmd);
    vfprintf(handle->gnucmd, cmd, ap);
    va_end(ap);

    fputs("\n", handle->gnucmd) ;
    fflush(handle->gnucmd) ;
    return ;
}


/**
  @brief    Plots a 2d graph from a list of doubles.
  @param    handle  Gnuplot session control handle.
  @param    d       Array of doubles.
  @param    n       Number of values in the passed array.
  @param    title   Title of the plot.
  @return   void

  Plots out a 2d graph from a list of doubles. The x-coordinate is the
  index of the double in the list, the y coordinate is the double in
  the list.

  Example:

  @code
    gnuplot_ctrl    *h ;
    double          d[50] ;
    int             i ;

    h = gnuplot_init() ;
    for (i=0 ; i<50 ; i++) {
        d[i] = (double)(i*i) ;
    }
    gnuplot_plot_x(h, d, 50, "parabola") ;
    sleep(2) ;
    gnuplot_close(h) ;
  @endcode
 */
void gnuplot_plot_x(gnuplot_ctrl *handle, uint64_t *d, int n, const char *title, unsigned char keep_last)
{
    int i;

    if (handle==NULL || d==NULL || (n<1))
        return;

    if(handle->file_no == 0)
    {
        handle->tmp0 = fopen(handle->tmp0name, "w");
        if (handle->tmp0 == NULL) {
            fprintf(stderr,"Cannot create temporary file: exiting plot");
            return;
        }
        handle->file_no = 1;
        for (i = 0; i < n; i++)
            fprintf(handle->tmp0, "%"PRIu64"\n", d[i]);
        fclose(handle->tmp0);
        handle->tmp0 = NULL;
        gnuplot_plot_atmpfile(handle,handle->tmp0name,title,keep_last);
    }
    else
    {
        handle->tmp1 = fopen(handle->tmp1name, "w");
        if (handle->tmp1 == NULL) {
            fprintf(stderr,"Cannot create temporary file: exiting plot");
            return;
        }
        handle->file_no = 0;
        for (i = 0; i < n; i++)
            fprintf(handle->tmp1, "%"PRIu64"\n", d[i]);
        fclose(handle->tmp1);
        handle->tmp1 = NULL;
        gnuplot_plot_atmpfile(handle,handle->tmp1name,title,keep_last);
    }
    return ;
}


/**
  @brief    Plot a 2d graph from a list of points.
  @param    handle      Gnuplot session control handle.
  @param    x           Pointer to a list of x coordinates.
  @param    y           Pointer to a list of y coordinates.
  @param    n           Number of doubles in x (assumed the same as in y).
  @param    title       Title of the plot.
  @return   void

  Plots out a 2d graph from a list of points. Provide points through a list
  of x and a list of y coordinates. Both provided arrays are assumed to
  contain the same number of values.

  @code
    gnuplot_ctrl    *h ;
    double          x[50] ;
    double          y[50] ;
    int             i ;

    h = gnuplot_init() ;
    for (i=0 ; i<50 ; i++) {
        x[i] = (double)(i)/10.0 ;
        y[i] = x[i] * x[i] ;
    }
    gnuplot_plot_xy(h, x, y, 50, "parabola") ;
    sleep(2) ;
    gnuplot_close(h) ;
  @endcode
 */
void gnuplot_plot_xy(gnuplot_ctrl *handle, double *x, double *y, int n, char *title,
                     unsigned char keep_last)
{
    int     i ;

    if (handle==NULL || x==NULL || y==NULL || (n<1)) return ;

    if(handle->file_no == 0)
    {
        handle->tmp0 = fopen(handle->tmp0name, "w");
        if (handle->tmp0 == NULL) {
            fprintf(stderr,"Cannot create temporary file: exiting plot");
            return;
        }
        handle->file_no = 1;
        for (i = 0; i < n; i++)
            fprintf(handle->tmp0, "%.18e %.18e\n", x[i], y[i]) ;
        fclose(handle->tmp0);
        handle->tmp0 = NULL;
        gnuplot_plot_atmpfile(handle,"handle->tmp0name",title,keep_last);
    }
    else
    {
        handle->tmp1 = fopen("handle->tmp1name", "w");
        if (handle->tmp1 == NULL) {
            fprintf(stderr,"Cannot create temporary file: exiting plot");
            return;
        }
        handle->file_no = 0;
        for (i = 0; i < n; i++)
            fprintf(handle->tmp1, "%.18e %.18e\n", x[i], y[i]) ;
        fclose(handle->tmp1);
        handle->tmp1 = NULL;
        gnuplot_plot_atmpfile(handle,"handle->tmp1name",title,keep_last);
    }
    return ;
}

