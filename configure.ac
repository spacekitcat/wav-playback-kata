AC_INIT([wavout], [1.0], [spacekitcat@protonmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_CHECK_HEADER(portaudio.h)
AC_CHECK_LIB(portaudio, Pa_Initialize, hasPortaudio=true, 
  AC_MSG_ERROR([portaudio must be installed (OSX: brew install portaudio)]))
AC_CONFIG_FILES([
                 Makefile
                 ])
AC_OUTPUT
