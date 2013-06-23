#!/usr/bin/python

"""GDB stop event handler.
Produces colored output on the stop event.
Output contains current registers, stack, local variables,
threads and source code.
Output can be redirected to the file instead of stdout and can be
viewed by in second terminal by "tail -f ~/gdbout.txt"

commands:
colored-display enable
colored-display disable
colored-display show                 -- show parameters
colored-display print                -- print data for current frame
colored-display output {file|stdout} -- set output stream
"""

import os, re, time, sys, termios, struct, fcntl, subprocess, datetime
import traceback

out_fname = os.environ['HOME'] + '/gdbout.txt'
out_file = None
enabled = False
print_to_file = False
has_colorizer = False
debug = 0

#====================== utils ==========================================

def format_header(s):
    s = '\033[1;31m=== ' + (s + ' ').ljust(80, '=') + '\033[0;0m\n'
    return s

def get_term_size():
    st = fcntl.ioctl(2, termios.TIOCGWINSZ, '1234')
    cr = struct.unpack('hh', st)
    return cr
    
def get_colorized_source(fname, linenum):
    rex = '^(0*)(%d):' % linenum
    re_curr_line = re.compile(rex, re.MULTILINE)
    cmd = ['source-highlight',
            '--line-number', '--line-number-ref=aaaa',
            '--line-range=%d-%d' % (linenum - 20, linenum + 20),
            '-o', '/dev/stdout',
            '-i', fname,
            '-f', 'esc']
    p = subprocess.Popen(cmd, stdout = subprocess.PIPE) 
    s = p.stdout.read()
    s = re_curr_line.sub('\033[1;33m\g<1>\g<2>=\033[0m', s)
    if p.wait() != 0:
        s += 'cannot execute source-highlight\n'
    return s
    
def get_plain_source(fname, linenum):
    rex = '^(0*)(%d):' % linenum
    re_curr_line = re.compile(rex, re.MULTILINE)
    f = open(fname, 'rt')
    s = f.read()
    f.close()
    lines = s.split('\n')
    lines = lines[linenum - 20 : linenum + 20]
    text = ''
    cnt = 0
    if linenum > 20:
        linenum = 20
    for line in lines:
        text += '%4d %s'
    return text

#=======================================================================

def dump_registers():
    s = format_header('registers')
    v = gdb.parse_and_eval('$rax')
    s += '$rax: 0x%x (%d)\n' % (v, v)
    return s

def dump_stack():
    frame = gdb.selected_frame()
    s = format_header('stack')
    cnt = 0
    while frame is not None:
        fn = frame.function()
        green = True
        if fn is None:
            if frame.name() is None:
                break
            s += str(cnt).ljust(3) + str(frame.name()).ljust(20)
        else:
            fname = fn.name

            # shorten long boost names

            n = fname.rfind('(')
            if n > 0:
                fname = fname[:n]

            if fname.startswith('boost::_bi::bind_t'):
                fname = 'bbind'
            elif fname.find('boost::') >= 0:
                green = False

            fname = fname.replace('boost::asio::', 'bas::')

            # compact long names
            if len(fname) > 60:
                fname = fname[:29] + '...' + fname[-28:]
            fname = fname.ljust(60)

            # hilight frame unless system lib
            if green:
                fname = '\033[32m' + fname + '\033[0m'
            s += str(cnt).ljust(3) + fname + ' ' \
                + os.path.basename(fn.symtab.filename) + ':' + str(fn.line)
        s += '\n'
        cnt += 1
        frame = frame.older()
    return s

def get_value_types(v):

    type = ''
    ctype = ''
    tag = ''

    if v.addr_class == gdb.SYMBOL_LOC_LABEL:
        type = 'l'
        ctype = ' '
    elif v.is_argument:
        type = 'a'
        if v.type.tag is not None:
            ctype = str(v.type.tag)
    elif v.is_variable:
        type = 'v'
    elif v.is_constant:
        type = 'c'
    elif v.type.code == gdb.TYPE_CODE_TYPEDEF:
        type = 't'

    if v.type.code == gdb.TYPE_CODE_STRUCT:
        tag = 'S'
    elif v.type.code == gdb.TYPE_CODE_UNION:
        tag = 'U'
    elif v.type.code == gdb.TYPE_CODE_ENUM:
        tag = 'E'

    if len(ctype) == 0:
        ctype = str(v.type)
    else:
        ctype = str(v.type.tag)

    if type == '':
        type = str(v.type.code)

    return type, ctype, tag

re_space = re.compile('\s+')

def format_value(x, frame):
    value = None

    type, ctype, tag = get_value_types(x)

    if x.is_argument or x.is_variable or x.is_constant:
        value = x.value(frame)

    # shorten some types
    if ctype == 'const astring &':
        ctype = 'c_astr&'
        value = value['_M_dataplus']['_M_p']
    elif ctype == 'astring &':
        ctype = 'astr&'
        value = value['_M_dataplus']['_M_p']
    elif ctype == 'astring':
        ctype = 'astr'
        value = value['_M_dataplus']['_M_p']
    elif ctype == 'std::locale::string':
        ctype = 'str'
        value = value['_M_dataplus']['_M_p']
    elif ctype.startswith('const boost::shared_ptr<'):
        ctype = ctype.replace('const boost::shared_ptr<', 'CBSP<')
        value = value['px'].dereference()
    elif ctype.startswith('boost::intrusive_ptr<'):
        ctype = ctype.replace('boost::intrusive_ptr<', 'BIP<')
        value = value['px'].dereference()
    elif ctype.startswith('std::_Rb_tree_iterator<'):
        ctype = ctype.replace('std::_Rb_tree_iterator<', 'iter<')
    elif ctype.find('tbb::internal::') >= 0:
        ctype = ctype.replace('tbb::internal::', '+tbi::')
        
    if value is None:
        value = ''
    else:
        try:
            value = str(value)
        except Exception:
            value = 'ERROR: ' + str(sys.exc_info())

    value = value.replace('\n', ' ')
    value = re_space.sub(' ', value)
    
    return type, ctype, tag, x.print_name, value

def dump_locals():
    s = format_header('locals')
    frame = gdb.selected_frame()
    try:
        block = frame.block()
    except:
        return s + 'no blocks in this frame\n'
    for x in block:
        type, ctype, tag, name, value = format_value(x, frame)
        s += '%s %s %s %s %s\n' % (\
            type, \
            name.ljust(10)[:10], \
            tag.ljust(2), \
            ctype.ljust(30)[:30], \
            value.ljust(40)[:40])
    return s + '\n'

def dump_threads():
    
    s = format_header('threads')
    
    inferior = gdb.selected_inferior()
    sel_thread = gdb.selected_thread() # save current thread
    sel_frame = gdb.selected_frame() # save current thread
    
    # switch to threads and get frames
    for x in inferior.threads():
        x.switch()
        frame = gdb.selected_frame()
        func = frame.function()
        name = ''
        fname = ''
        if func is not None:
            name = func.name
            if func.symtab is not None:
                fname = os.path.basename(func.symtab.filename)
                fname += ':' + str(func.line)
        else:
            name = frame.name()

        row = '%2d %4d %4d %s %s' % (\
            x.num, \
            x.ptid[0], \
            x.ptid[1], \
            name, \
            fname
            )

        if x.ptid == sel_thread.ptid:
            row = '\033[1;33m' + row + '\033[0m'
        s += row + '\n'
    
    sel_thread.switch() # restore thread
    sel_frame.select()  # restore frame
    
    return s + '\n'


re_where = re.compile(' at ([^:]*):(\d+)$')

def get_source_line():
    s = gdb.execute('frame', to_string = True)
    m = re_where.search(s.split('\n')[0])
    if m is None:
        return '', 0
    return m.group(1), int(m.group(2))

def display_data(file):
    try:
        start = datetime.datetime.now()
        s = '\033[2J\033[;H' # clear screen
        (rows, cols) = get_term_size()
        fname, line = get_source_line()
        if line > 0:
            if has_colorizer:
                s += get_colorized_source(fname, line)
            else:
                s += get_plain_source(fname, line)
        s += dump_registers()
        s += dump_stack()
        s += dump_locals()
        s += dump_threads()
        lines = s.split('\n')
        print >> file, s
        end = datetime.datetime.now()
        print >> file, 'time: ' + str(end - start)
    except Exception:
        traceback.print_tb(sys.exc_info()[2])
        print >> file, 'ERROR: ' + str(sys.exc_info())
    file.flush()

def parse_annot(text, file):

    lines = text.split('\n')
    state = ''
    depth = 0
    arr_value = ''
    value = ''
    name = ''

    for line in lines:
        s = line.lstrip('\x1a\x1a')
     
        if s.startswith('field-begin'):
            state = s
            depth += 1
            continue
       
        if s.startswith('field-value'):
            state = s
            continue

        if s.startswith('field-name-end'):
            state = s
            continue
     
        if s.startswith('value-history-begin'):
            continue
       
        if s.startswith('value-history-value'):
            state = s
            continue
       
        if s.startswith('value-history-end'):
            state = s
            continue
            
        if s.startswith('array-section-begin'):
            arr_value = ''
            continue

        if s.startswith('array-section-end'):
            value = arr_value
            state = s
            continue

        if s.startswith('elt'):
            arr_value += value + ' '
#            print 'arr_value:', arr_value
            continue

        if s.startswith('field-end'):
            state = s
            depth -= 1
            continue

        # print 'state:', state, ',', s

        if state == 'field-begin -':
            name = s
        elif state == 'field-value':
            value = s
        elif state == 'value-history-value':
            if s != '':
                value = s
        elif state == 'field-end' and name != '':
            print >> file, ' ' * (depth * 2) + name, '=', value
            name = ''
            value = ''
            arr_value = ''
        elif state == 'value-history-end':
            print >> file, value
            name = ''
            value = ''
            arr_value = ''


def stop_event_handler(event):
    display_data(out_file)


def disable():
    global enabled
    global out_file
    if not enabled:
        gdb.execute('echo \033[31m=== colored-display already is off ===\033[0m\n')
        return
    gdb.events.stop.disconnect(stop_event_handler)
    enabled = False
    out_file.close()
    out_file = None
    gdb.execute('echo \033[31m=== colored-display is off ===\033[0m\n')

    
def enable():
    global enabled
    global out_file
    global has_colorizer
    if enabled:
        gdb.execute('echo \033[31m=== colored-display is enabled already. Disabling first ===\033[0m\n')
        disable()

    if print_to_file:
        out_file = open(out_fname, 'wt')
    else:
        out_file = sys.stdout
        gdb.execute('set pagination off')
    gdb.events.stop.connect(stop_event_handler)
    enabled = True
    has_colorizer = (os.system('which source-highlight') == 0)
    print '\033[31m=== colored-display is on ===\033[0m'
    if print_to_file:
        print 'printing to file ' + out_fname
    else:
        print 'printing to stdout'

def debug():
    global debug
    debug = 1
    f  = open('/home/sergv/tmp/1.txt')
    text = f.read()
    parse_annot(text, sys.stdout)

if __name__ == '__main__':
    debug()
    sys.exit(1)
    
import gdb

class main_command(gdb.Command):
    """
    enable auto displaying
    """

    def __init__(self):
        gdb.Command.__init__(self, "colored-display", gdb.COMMAND_DATA, prefix=True)

    def invoke(self, arg, from_tty):
        print arg

main_command()

class enable_command(gdb.Command):
    """
    enable colored output
    """

    def __init__(self):
        gdb.Command.__init__(self, "colored-display enable", gdb.COMMAND_DATA)

    def invoke(self, arg, from_tty):
        enable()

enable_command()

class set_output(gdb.Command):
    """
    set output whether to stdout or to file
    """

    def __init__(self):
        gdb.Command.__init__(self, "colored-display output", gdb.COMMAND_DATA)

    def invoke(self, arg, from_tty):
        global print_to_file
        print_to_file = (arg == 'file')
        enable()

set_output()

class disable(gdb.Command):
    """
    disable colored output
    """

    def __init__(self):
        gdb.Command.__init__(self, "colored-display disable", gdb.COMMAND_DATA)

    def invoke(self, arg, from_tty):
        disable()

disable()

class display(gdb.Command):
    """
    print registers, stack and locals
    """

    def __init__(self):
        gdb.Command.__init__(self, "colored-display print", gdb.COMMAND_DATA)

    def invoke(self, arg, from_tty):
        if not enabled:
            print 'colored-display is not enabled'
            return
        if arg != '':
            s = gdb.execute('p ' + arg, from_tty = True, to_string = True)
#            f = open('/home/sergv/tmp/1.txt', 'wb')
#            f.write(s)
#            f.close()
            print >> out_file, '\033[1;31m=== ' + (arg + ' ').ljust(80, '=') + '\033[0;0m'
            parse_annot(s, out_file)
            out_file.flush()
        else:
            display_data(out_file)

display()

class show(gdb.Command):
    """
    show current status
    """

    def __init__(self):
        gdb.Command.__init__(self, "colored-display show", gdb.COMMAND_DATA)

    def invoke(self, arg, from_tty):
        if enabled:
            print 'colored-display is on'
        else:
            print 'colored-display is off'
        if os.system('which source-highlight') == 0:
            print 'source-highlight is installed'
        else:
            print 'source-highlight is not installed'
        if print_to_file:
            print 'printing to file ' + out_fname
        else:
            print 'printing to stdout'

show()

