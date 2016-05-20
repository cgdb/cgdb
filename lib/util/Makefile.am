noinst_LIBRARIES = libcgdbutil.a

libcgdbutil_a_SOURCES = \
    fork_util.cpp \
    fork_util.h \
    fs_util.cpp \
    fs_util.h \
    io.cpp \
    io.h \
    logger.cpp \
    logger.h \
    pseudo.cpp \
    pseudo.h \
    sys_util.cpp \
    sys_util.h \
    terminal.cpp \
    terminal.h

noinst_PROGRAMS = cgdbutil_driver

cgdbutil_driver_LDFLAGS = \
    -L$(top_builddir)/lib/util

cgdbutil_driver_LDADD = \
    libcgdbutil.a

cgdbutil_driver_SOURCES = driver.cpp
