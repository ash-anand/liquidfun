/*
 * freeglut_state_mswin.c
 *
 * The Windows-specific mouse cursor related stuff.
 *
 * Copyright (c) 2012 Stephen J. Baker. All Rights Reserved.
 * Written by John F. Fay, <fayjf@sourceforge.net>
 * Creation date: Sun Jan 22, 2012
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PAWEL W. OLSZTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <GL/freeglut.h>
#include "freeglut_internal_mswin.h"


extern GLboolean fgSetupPixelFormat( SFG_Window* window, GLboolean checkOnly,
                                     unsigned char layer_type );

/* 
 * Helper functions for getting client area from the window rect
 * and the window rect from the client area given the style of the window
 * (or a valid window pointer from which the style can be queried).
 */
extern RECT fghGetClientArea( const SFG_Window *window, BOOL wantPosOutside );
extern void fghGetBorderWidth(const DWORD windowStyle, int* xBorderWidth, int* yBorderWidth);


/* The following include file is available from SGI but is not standard:
 *   #include <GL/wglext.h>
 * So we copy the necessary parts out of it to support the multisampling query
 */
#define WGL_SAMPLES_ARB                0x2042



int fgPlatformGlutGet ( GLenum eWhat )
{
    int returnValue ;
    GLboolean boolValue ;

    int nsamples = 0;

    switch( eWhat )
    {
    case GLUT_WINDOW_NUM_SAMPLES:
      glGetIntegerv(WGL_SAMPLES_ARB, &nsamples);
      return nsamples;

    /* Handle the OpenGL inquiries */
    case GLUT_WINDOW_RGBA:
#if defined(_WIN32_WCE)
      boolValue = (GLboolean)0;  /* WinCE doesn't support this feature */
#else
      glGetBooleanv ( GL_RGBA_MODE, &boolValue );
      returnValue = boolValue ? 1 : 0;
#endif
      return returnValue;
    case GLUT_WINDOW_DOUBLEBUFFER:
#if defined(_WIN32_WCE)
      boolValue = (GLboolean)0;  /* WinCE doesn't support this feature */
#else
      glGetBooleanv ( GL_DOUBLEBUFFER, &boolValue );
      returnValue = boolValue ? 1 : 0;
#endif
      return returnValue;
    case GLUT_WINDOW_STEREO:
#if defined(_WIN32_WCE)
      boolValue = (GLboolean)0;  /* WinCE doesn't support this feature */
#else
      glGetBooleanv ( GL_STEREO, &boolValue );
      returnValue = boolValue ? 1 : 0;
#endif
      return returnValue;

    case GLUT_WINDOW_RED_SIZE:
      glGetIntegerv ( GL_RED_BITS, &returnValue );
      return returnValue;
    case GLUT_WINDOW_GREEN_SIZE:
      glGetIntegerv ( GL_GREEN_BITS, &returnValue );
      return returnValue;
    case GLUT_WINDOW_BLUE_SIZE:
      glGetIntegerv ( GL_BLUE_BITS, &returnValue );
      return returnValue;
    case GLUT_WINDOW_ALPHA_SIZE:
      glGetIntegerv ( GL_ALPHA_BITS, &returnValue );
      return returnValue;
    case GLUT_WINDOW_ACCUM_RED_SIZE:
#if defined(_WIN32_WCE)
      returnValue = 0;  /* WinCE doesn't support this feature */
#else
      glGetIntegerv ( GL_ACCUM_RED_BITS, &returnValue );
#endif
      return returnValue;
    case GLUT_WINDOW_ACCUM_GREEN_SIZE:
#if defined(_WIN32_WCE)
      returnValue = 0;  /* WinCE doesn't support this feature */
#else
      glGetIntegerv ( GL_ACCUM_GREEN_BITS, &returnValue );
#endif
      return returnValue;
    case GLUT_WINDOW_ACCUM_BLUE_SIZE:
#if defined(_WIN32_WCE)
      returnValue = 0;  /* WinCE doesn't support this feature */
#else
      glGetIntegerv ( GL_ACCUM_BLUE_BITS, &returnValue );
#endif
      return returnValue;
    case GLUT_WINDOW_ACCUM_ALPHA_SIZE:
#if defined(_WIN32_WCE)
      returnValue = 0;  /* WinCE doesn't support this feature */
#else
      glGetIntegerv ( GL_ACCUM_ALPHA_BITS, &returnValue );
#endif
      return returnValue;
    case GLUT_WINDOW_DEPTH_SIZE:
      glGetIntegerv ( GL_DEPTH_BITS, &returnValue );
      return returnValue;

    case GLUT_WINDOW_BUFFER_SIZE:
      returnValue = 1 ;                                      /* ????? */
      return returnValue;
    case GLUT_WINDOW_STENCIL_SIZE:
      returnValue = 0 ;                                      /* ????? */
      return returnValue;

    case GLUT_WINDOW_X:
    case GLUT_WINDOW_Y:
    case GLUT_WINDOW_WIDTH:
    case GLUT_WINDOW_HEIGHT:
    {
        /*
         *  There is considerable confusion about the "right thing to
         *  do" concerning window  size and position.  GLUT itself is
         *  not consistent between Windows and UNIX/X11; since
         *  platform independence is a virtue for "freeglut", we
         *  decided to break with GLUT's behaviour.
         *
         *  Under UNIX/X11, it is apparently not possible to get the
         *  window border sizes in order to subtract them off the
         *  window's initial position until some time after the window
         *  has been created.  Therefore we decided on the following
         *  behaviour, both under Windows and under UNIX/X11:
         *  - When you create a window with position (x,y) and size
         *    (w,h), the upper left hand corner of the outside of the
         *    window is at (x,y) and the size of the drawable area  is
         *    (w,h).
         *  - When you query the size and position of the window--as
         *    is happening here for Windows--"freeglut" will return
         *    the size of the drawable area--the (w,h) that you
         *    specified when you created the window--and the coordinates
         *    of the upper left hand corner of the drawable
         *    area--which is NOT the (x,y) you specified.
         */

        RECT winRect;

        freeglut_return_val_if_fail( fgStructure.CurrentWindow != NULL, 0 );

#if defined(_WIN32_WCE)
        GetWindowRect( fgStructure.CurrentWindow->Window.Handle, &winRect );
#else
        winRect = fghGetClientArea(fgStructure.CurrentWindow, FALSE);
#endif /* defined(_WIN32_WCE) */

        switch( eWhat )
        {
        case GLUT_WINDOW_X:      return winRect.left                ;
        case GLUT_WINDOW_Y:      return winRect.top                 ;
        case GLUT_WINDOW_WIDTH:  return winRect.right - winRect.left;
        case GLUT_WINDOW_HEIGHT: return winRect.bottom - winRect.top;
        }
    }
    break;

    case GLUT_WINDOW_BORDER_WIDTH :
    case GLUT_WINDOW_HEADER_HEIGHT :
#if defined(_WIN32_WCE)
        return 0;
#else
        {
            DWORD windowStyle;

            if (fgStructure.CurrentWindow && fgStructure.CurrentWindow->Window.Handle)
                windowStyle = GetWindowLong(fgStructure.CurrentWindow->Window.Handle, GWL_STYLE);
            else
                /* If no window, return sizes for a default window with title bar and border */
                windowStyle = WS_OVERLAPPEDWINDOW;
            
            switch( eWhat )
            {
            case GLUT_WINDOW_BORDER_WIDTH:
                {
                    int xBorderWidth, yBorderWidth;
                    fghGetBorderWidth(windowStyle, &xBorderWidth, &yBorderWidth);
                    return xBorderWidth;
                }
            case GLUT_WINDOW_HEADER_HEIGHT:
                /* Need to query for WS_MAXIMIZEBOX to see if we have a title bar, the WS_CAPTION query is also true for a WS_DLGFRAME only... */
                return (windowStyle & WS_MAXIMIZEBOX)? GetSystemMetrics( SM_CYCAPTION ) : 0;
            }
        }
#endif /* defined(_WIN32_WCE) */

    case GLUT_DISPLAY_MODE_POSSIBLE:
#if defined(_WIN32_WCE)
        return 0;
#else
        return fgSetupPixelFormat( fgStructure.CurrentWindow, GL_TRUE,
                                    PFD_MAIN_PLANE );
#endif /* defined(_WIN32_WCE) */


    case GLUT_WINDOW_FORMAT_ID:
#if !defined(_WIN32_WCE)
        if( fgStructure.CurrentWindow != NULL )
            return GetPixelFormat( fgStructure.CurrentWindow->Window.Device );
#endif /* defined(_WIN32_WCE) */
        return 0;

    default:
        fgWarning( "glutGet(): missing enum handle %d", eWhat );
        break;
    }

	return -1;
}


int fgPlatformGlutDeviceGet ( GLenum eWhat )
{
    switch( eWhat )
    {
    case GLUT_HAS_KEYBOARD:
        /*
         * Win32 is assumed a keyboard, and this cannot be queried,
         * except for WindowsCE.
         *
         * X11 has a core keyboard by definition, although it can
         * be present as a virtual/dummy keyboard. For now, there
         * is no reliable way to tell if a real keyboard is present.
         */
#if defined(_WIN32_CE)
        return ( GetKeyboardStatus() & KBDI_KEYBOARD_PRESENT ) ? 1 : 0;
#   if FREEGLUT_LIB_PRAGMAS
#       pragma comment (lib,"Kbdui.lib")
#   endif

#else
        return 1;
#endif

    case GLUT_HAS_MOUSE:
        /*
         * MS Windows can be booted without a mouse.
         */
        return GetSystemMetrics( SM_MOUSEPRESENT );

    case GLUT_NUM_MOUSE_BUTTONS:
#  if defined(_WIN32_WCE)
        return 1;
#  else
        return GetSystemMetrics( SM_CMOUSEBUTTONS );
#  endif

    default:
        fgWarning( "glutDeviceGet(): missing enum handle %d", eWhat );
        break;
    }

    /* And now -- the failure. */
    return -1;
}


int fgPlatformGlutLayerGet( GLenum eWhat )
{
    /*
     * This is easy as layers are not implemented ;-)
     *
     * XXX Can we merge the UNIX/X11 and WIN32 sections?  Or
     * XXX is overlay support planned?
     */
    switch( eWhat )
    {

    case GLUT_OVERLAY_POSSIBLE:
/*      return fgSetupPixelFormat( fgStructure.CurrentWindow, GL_TRUE,
                                   PFD_OVERLAY_PLANE ); */
      return 0 ;

    case GLUT_LAYER_IN_USE:
        return GLUT_NORMAL;

    case GLUT_HAS_OVERLAY:
        return 0;

    case GLUT_TRANSPARENT_INDEX:
        /*
         * Return just anything, which is always defined as zero
         *
         * XXX HUH?
         */
        return 0;

    case GLUT_NORMAL_DAMAGED:
        /* XXX Actually I do not know. Maybe. */
        return 0;

    case GLUT_OVERLAY_DAMAGED:
        return -1;

    default:
        fgWarning( "glutLayerGet(): missing enum handle %d", eWhat );
        break;
    }

    /* And fail. That's good. Programs do love failing. */
    return -1;
}



