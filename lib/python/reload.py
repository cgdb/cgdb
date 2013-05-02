import gdb

class reload(gdb.Command):
    """
    reload commands (for debugging)
    """

    def __init__(self):
        gdb.Command.__init__(self, "aa", gdb.COMMAND_DATA)

    def invoke(self, arg, from_tty):
        gdb.execute('source ~/local/share/gdb/python/gdb/command/colored_display.py')
        gdb.execute('colored-display output file')
        print '~/local/share/gdb/python/gdb/command/colored_display.py reloaded'

reload()
