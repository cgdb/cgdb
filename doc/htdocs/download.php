<?
    define('PAGE_ID', "download_page");
    define('LATEST', "0.6.5");

    include "header.php";
?>

    <h3>Downloading CGDB</h3>

    <div class="pagetext">
      <p>Choose your preferred installation mechanism:</p>
      <ul>
        <li>
          <a href="#packages">Binary packages (platform-specific)</a>
        </li>
        <li>
          <a href="#tarball">Source tarball</a>
        </li>
        <li>
          <a href="#subversion">Subversion repository access</a>
        </li>
      </ul>
    </div>

    <a name="packages"></a>
    <h4>Binary Packages</h4>

    <div class="pagetext">
      <p>
        The following distributions offer native packages for CGDB.
        If you know of another distribution that packages CGDB, please let us
        know so we can update this page.
      </p>
    </div>

    <!-- Nested pagetext divs as kind of a hack to get the whole page
         indented below this point -->
    <div class="pagetext">

      <h5>Debian</h5>

      <div class="pagetext">
        <p>
          Debian's package system contains a
          <a href="http://packages.qa.debian.org/c/cgdb.html">CGDB package</a>,
          maintained by
          <a href="http://www.semistable.com/debian.html">Robert Lemmen</a>.
          The stable version is fairly recent (0.6.3 as of this writing),
          testing will get you the latest (<?= LATEST ?>).
        </p>

        <p>
          To install, as root:
          <code>apt-get install cgdb</code>
        </p>
      </div>

      <h5>Ubuntu</h5>

      <div class="pagetext">
        <p>
          See Debian instructions, or if you prefer aptitude:
          <code>sudo aptitude install cgdb</code>
        </p>
      </div>

      <h5>Gentoo</h5>

      <div class="pagetext">
        <p>
          Portage generally has a relatively recent version of CGDB (<a
          href="http://www.gentoo-portage.com/dev-util/cgdb">dev-util/cgdb</a>).
          As of this writing, 0.6.2 is unmasked, but 0.6.3 is masked with
          ~keyword.  (They're both stable, Gentoo just doesn't keep up so well.)
        </p>

        <p>
          To install, as root:
          <code>emerge -av cgdb</code>
        </p>

        <p>
          We also provide a 
          <a href="files/cgdb-<?= LATEST ?>.ebuild">current Gentoo ebuild</a>
          (<?= LATEST ?>)
          that you can use instead.  If you've never installed an ebuild before,
          check out the <a href=
          "http://gentoo-wiki.com/HOWTO_Installing_3rd_Party_Ebuilds">Gentoo
          Wiki on 3rd party ebuilds</a>.  Once you overlay the new ebuild,
          install cgdb with the command above.
        </p>
      </div>

      <h5>Fedora</h5>

      <div class="pagetext">
        <p>
          Fedora's extra repository has an up-to-date CGDB package.
          To install it, as root:
          <code>yum install cgdb</code>
        </p>
      </div>

      <h5>Red Hat/CentOS (5.0)</h5>

      <div class="pagetext">
        <p>
          <a href="http://fedoraproject.org/wiki/EPEL">Fedora EPEL</a>
          repository has an up-to-date CGDB package. To install it, add
          <a href="http://fedoraproject.org/wiki/EPEL/FAQ#howtouse">EPEL
          support</a> to your existing RedHat/CentOS installation, then
          install as you would in Fedora (above).
        </p>
      </div>

      <h5>Windows</h5>

      <div class="pagetext">
        <p>
          Windows users can use this
          <a href="files/cgdb-<?= LATEST ?>-win32.tar.gz">win32 binary</a>,
          which will work in a command window or a Cygwin terminal.
          For Cygwin users, you may need to delete the DLLs in the
          'bin' directory.
        </p>
      </div>

      <h5>MacOS X</h5>

      <div class="pagetext">
        <p>
          You can get a CGDB Mac PPC binary by using Jens Frederich's prebuilt
          <a href="files/cgdb-<?= LATEST ?>-osx-universal.tar.gz">universal
          binary</a>, also available as a
          <a href="files/cgdb-<?= LATEST ?>-osx-universal.dmg">disk image</a>.
          If you would like to contact him, his address is jfrederich (at)
          gmail (dot) com.  Thanks Jens!
        </p>

        <p>
          Alternatively, CGDB may be installed via the
          <a href="http://www.macports.org/">MacPorts</a> package system.
          Install MacPorts, then run:
          <code>
            sudo port install cgdb
          </code>
        </p>
      </div>
    </div>

    <a name="tarball"></a>
    <h4>Install from Sources</h4>

    <div class="pagetext">
      <h5>Prerequisites</h5>

      <ol>
        <li>
          <a
          href="http://tiswww.case.edu/~chet/readline/rltop.html">readline</a>
          5.1 or greater.  An important design problem was fixed in version
          5.1, any older version will not support CGDB.
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
        <a href="http://cgdb.svn.sourceforge.net/viewvc/cgdb/cgdb/trunk/ChangeLog?view=markup">ChangeLog</a> -
        <a href="http://cgdb.svn.sourceforge.net/viewvc/cgdb/cgdb/trunk/NEWS?view=markup">NEWS</a>
        </p>
      </div>
    </div>

    <a name="subversion"></a>
    <h4>Subversion Access</h4>

      <div class="pagetext">

        <p>
          The most recent changes to CGDB can be found by checking out
          a copy of the current
          <a href="http://subversion.tigris.org/">Subversion</a> tree.  You may
          want to try this to see if a bug is resolved before reporting the
          error. It should be noted that the svn tree is not guaranteed to
          be stable and is not tested as well as the latest stable release.
        </p>

        <p>
          To get a copy of the svn repository, use the following command:
          <code>
            svn co https://cgdb.svn.sourceforge.net/svnroot/cgdb/cgdb/trunk cgdb
          </code>
          After checking out the tree, you will need to generate the configure
          script before you can configure and build the code.  Use the script
          "autoregen.sh" at the top level of the source tree to do this
          automatically.
        </p>

        <p>
          <b>Note:</b> Building the code checked out from svn will require
          additional tools installed to generate the autoconf script.  We use
          the following versions, although others may work for you:
        </p>

        <ul>
          <li>aclocal  (GNU automake)   1.9.6</li>
          <li>autoconf (GNU Autoconf)   2.59</li>
          <li>m4       (GNU m4)         1.4.4</li>
        </ul>

        <p>
          Finally, if you decide to modify the tokenizer library or the config
          file module, you will need to have flex installed.  We have used flex
          2.5.4 to build cgdb.
        </p>

      </div>

<?
    include "footer.php";
?>
