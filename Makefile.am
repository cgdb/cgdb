# The difference between SUBDIRS and DIST_SUBDIRS is that the
# readline directory simply needs to be copied as is, where as
# the rest of the directories have a make dist rule run on them
AUTOMAKE_OPTIONS = -Wportability foreign

if WANT_TESTS
    MAYBE_TESTS = tests
endif

SUBDIRS=lib cgdb doc test $(MAYBE_TESTS)
DIST_SUBDIRS=lib cgdb doc test $(MAYBE_TESTS)
