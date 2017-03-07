noinst_LIBRARIES = libcgdbutil.a

libcgdbutil_a_SOURCES = \
	clog.h \
    cgdb_clog.cpp \
    cgdb_clog.h \
    fork_util.cpp \
    fork_util.h \
    fs_util.cpp \
    fs_util.h \
    io.cpp \
    io.h \
    pseudo.cpp \
    pseudo.h \
    sys_util.cpp \
    sys_util.h \
    sys_win.cpp \
    sys_win.h \
    terminal.cpp \
    terminal.h

noinst_PROGRAMS = cgdbutil_driver

cgdbutil_driver_LDFLAGS = \
    -L$(top_builddir)/lib/util

cgdbutil_driver_LDADD = \
    libcgdbutil.a

cgdbutil_driver_SOURCES = driver.cpp
