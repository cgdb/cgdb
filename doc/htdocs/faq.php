<?
    define('PAGE_ID', "documentation_page");

    include "header.php";
?>

    <h3>Frequently Asked Questions</h3>

    <div class="pagetext">

    <ol>
      <li><a href="#is_CGDB">What is CGDB?</a>
      <li><a href="#get_CGDB">Where can I get CGDB?</a>
      <li><a href="#OS_CGDB">What platforms does CGDB support?</a>
      <li><a href="#features">When will feature X be added?</a>
      <li><a href="#distro">Will CGDB be packaged with my favorite Linux
                            distribution?</a>
      <li><a href="#versions">What versions of GDB are supported?</a>
      <li><a href="#whouses">Who uses CGDB?</a>
      <li><a href="#automate_CGDB">How do I automate colon commands using the cgdbrc file?</a>
      <li><a href="#keys_CGDB">Why do some keys not work properly for me in CGDB?</a>
      <li><a href="#tty_CGDB">Why does CGDB not work when a tty can not be opened?</a>
    </ol>

    <a name="is_CGDB"></a>
    <h4>What is CGDB?</h4>

    <p>
    CGDB is a curses-based interface to the GNU Debugger (GDB).
    The goal of CGDB is to be lightweight and responsive; not encumbered with
    unnecessary features. The interface is designed to deliver the familiar
    GDB text interface, with a split screen showing the source as it executes.
    The UI is modeled on the classic Unix text editor, vi. Those familiar with
    vi should feel right at home using CGDB.
    </p>

    <a name="get_CGDB"></a>
    <h4>Where can I get CGDB?</h4>

    <p>
    The CGDB project is hosted on http://sourceforge.net/. You can view the
    project by going to http://sourceforge.net/projects/cgdb/. Alternatively,
    CGDB's home-page is at http://cgdb.sourceforge.net/.
    If you would like to download CGDB, please click on the download link to
    the left.
    </p>

    <a name="OS_CGDB"></a>
    <h4>What platforms does CGDB support?</h4>

    <p>
    CGDB is used daily on Linux, Solaris and Cygwin (WinNT, 2000, and XP).
    It has been reported to work on AIX, FreeBSD, HURD and Mac OS X.
    </p>

    <a name="features"></a>
    <h4>When will feature X be added?</h4>

    <p>
    Great question.  We're not sure.
    </p>

    <a name="distro"></a>
    <h4>Will CGDB be packaged with my favorite Linux distribution?</h4>

    <p>
    Currently CGDB is available on Debian, Gentoo, and Mac OS X.  See the
    download page for more details.  If you would like to see a package
    for a different distribution, you are welcome to make one. :)
    </p>

    <a name="versions"></a>
    <h4>What versions of GDB are supported?</h4>

    <p>
    This is an area where CGDB shines -- CGDB has been tested successfully
    on versions of GDB as old as 4.17 (Gnat's patched GDB).  It supports all
    recent releases up to the current 6.4.  We will continue to test it to
    ensure it continues to support the latest GDB.  Unlike some frontends,
    CGDB does not require a patched GDB, or a special version of any kind.
    </p>

    <a name="whouses"></a>
    <h4>Who uses CGDB?</h4>

    <p>
    We're excited to see that many people are catching onto the project
    even in its current immature state.  We've been contacted by people
    worldwide (US, France, Germany, Poland, India, China, Japan) about it!
    We hope that it can become the de facto text GDB front end, at least for
    Vim fans. :) (Emacs users can check into the TUI, which is a compile-time
    option that can be build into GDB.)
    </p>

    <a name="automate_CGDB"></a>
    <h4>How do I automate colon commands using the cgdbrc file?</h4>

    <p>
    CGDB looks at the file $HOME/.cgdb/cgdbrc. It executes each of the lines
    in that file in order as if they were typed into the status bar. This is
    a very useful way of automating commands.
    </p>

    <a name="keys_CGDB"></a>
    <h4>Why do some keys not work properly for me in CGDB?</h4>

    <p>
    CGDB is proud of its custom key input library. This is because
    it *can* use the ESC key, without having problems understanding
    escape sequences that are generated when certain keys are hit.
    Some of the keys that generate escape sequences are the arrow keys,
    page up, page down, insert, delete, home, end and all the function
    keys.
    </p>

    <p>
    If you have problems using any of the keys above, you should modify
    the value of escdelay. Please look at the README file for more
    information on escdelay.
    </p>

    <a name="tty_CGDB"></a>
    <h4>Why does CGDB not work when a tty can not be opened?</h4>
    <p>
    There are 2 problems with this.
    </p>

    <ol>
      <li>
        <p>
        The annotate 2 communication protocol used between CGDB and GDB is
        lacking. It is difficult (at best) to figure out which output is GDB
        and which output is the inferior program. It is impossible to send input
        to the inferior program without error.
        </p>

        <p>
        To solve this CGDB used the tty command to be able to allow the user to
        send data to the inferior program. It also can easily determine which
        data is from the inferior and which data is from GDB.
        </p>
      </li>
      <li>
        <p>
        Readline can not be used if there is no tty available.
        </p>

        <p>
        All in all, not allowing CGDB to work when there is no tty is a bug. In
        the future, when CGDB will work when no tty is available, there will be
        several features that do not work:
        <ol>
          <li>
            No sending input to the inferior program. The workaround is to
            start the program from the terminal and attach to it using GDB.
          </li>
          <li>
            Readline will not work.
          </li>
        </ol>
        </p>
      </li>
    </ol>
    
    </div>
<?
    include "footer.php";
?>
