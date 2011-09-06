<?
    define('PAGE_ID', "download_page");
    define('LATEST', "0.6.5");

    include "header.php";
?>

    <h3>Downloading cgdb</h3>

    <h4>Binary Packages</h4>
    <div class="pagetext">
    <p>
      <b>Unix:</b> Normally you can install a stable version of cgdb through
      your distribution's packaging system.  Packages are available in all
      major Linux distributions (Arch, CentOS, Debian, Fedora, Gentoo, RedHat,
      Ubuntu, and probably others), as well as in the MacPorts system for
      MacOS X.
    </p>
    <p>
      <b>Windows binary:</b> <a href="files/cgdb-<?= LATEST ?>-win32.tar.gz"
      >cgdb-<?= LATEST ?>-win32.tar.gz</a>.
    </p>
    <p>
      This is a Cygwin executable that will run in a command window
      or a Cygwin terminal.  Cygwin users may need to delete the DLLs in
      the 'bin' directory.
    </p>
    </div>

    <h4>Source Tarball</h4>

    <div class="pagetext">
      <h5>Prerequisites</h5>
      <ol>
        <li>
          <a
          href="http://tiswww.case.edu/~chet/readline/rltop.html">readline</a>
          5.1 or greater.  An important design problem was fixed in version
          5.1, any older version will not support cgdb.
        </li>
        <li>
          <a
          href="http://www.gnu.org/software/ncurses/ncurses.html">ncurses</a>
          (or curses, but it doesn't always look pretty), any recent version
          (i.e. 5+) will do.
        </li>
      </ol>

      <h5>Stable Release:</h5>
      <div class="pagetext">
        <p>
        <a href="http://prdownloads.sourceforge.net/cgdb/cgdb-<?= LATEST ?>.tar.gz?download">cgdb-<?= LATEST ?>.tar.gz</a> -
        <a href="http://cgdb.git.sourceforge.net/git/gitweb.cgi?p=cgdb/cgdb;a=blob;f=NEWS;hb=HEAD">NEWS</a>
        </p>
      </div>
    </div>

    <a name="git"></a>
    <h4>Git Repository</h4>

    <div class="pagetext">
      <p>
        cgdb's <a href="http://git-scm.com/">git</a> repository is available
        for development purposes, and may be less stable than official
        releases of cgdb.
      </p>
      <p>
        Git (anonymous) download command:
        <code>
          git clone git://cgdb.git.sourceforge.net/gitroot/cgdb/cgdb
        </code>
        After checking out the tree, you will need to generate the configure
        script before you can configure and build the code.  Run
        "autogen.sh" at the top level of the source tree to do this
        automatically.
      </p>
      <p>
        <b>Note:</b> Building the code checked out from git will require
        additional tools installed to generate the autoconf script.  We use
        the following versions, although others may work for you:
      </p>
      <ul>
        <li>automake (GNU automake)   1.11.1</li>
        <li>autoconf (GNU Autoconf)   2.67</li>
        <li>m4       (GNU m4)         1.4.14</li>
      </ul>
      <p>
        Finally, if you decide to modify the tokenizer library or the config
        file module, you will need to have flex installed.  We have used flex
        2.5.35 to build cgdb.
      </p>
    </div>

<?
    include "footer.php";
?>
