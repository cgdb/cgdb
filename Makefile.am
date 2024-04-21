# The difference between SUBDIRS and DIST_SUBDIRS is that the
# readline directory simply needs to be copied as is, where as
# the rest of the directories have a make dist rule run on them
AUTOMAKE_OPTIONS = -Wportability foreign
SUBDIRS=lib cgdb testsuite
DIST_SUBDIRS=lib cgdb testsuite

if WANT_DOCS
SUBDIRS += doc
DIST_SUBDIRS += doc
endif
