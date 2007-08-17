<?
    define('PAGE_ID', "download_page");

    include "header.php";
?>

    <h3>Downloading CGDB</h3>

    <div class="pagetext">
      <p>
      You may be able to get a binary of CGDB from your favorite distribution.
      Currently, the below distribution's offer packaged binaries of CGDB. If
      you know of another distro that packages CGDB, please let us know so we
      can add a link.
      </p>

      <h4>Debian Users</h4>

      <p>
      Debian's package system contains a
      <a href="http://packages.qa.debian.org/c/cgdb.html">CGDB package</a>,
      maintained by
      <a href="http://www.semistable.com/debian.html">Robert Lemmen</a>.
      The stable version is fairly recent (0.6.3 as of this writing),
      testing will get you the latest (0.6.4).  You can always see what
      versions are available
      <a href="http://packages.qa.debian.org/c/cgdb.html">here</a>.
      </p>

      <h4>Mac OS X Users</h4>

      <p>
        <em>
          <b>Update:</b> Currently unavailable, we're working to
          reach the maintainer.
        </em>
      </p>

      <p>
      You can get a CGDB Mac PPC binary by using Jens Frederich's prebuilt
      static binary. This is beneficial, since he has done all the work of
      finding the correct readline version and statically linking it to CGDB.
      You can see the builds he has available
      <a href="http://codingsaloon.com/cs/">here</a>.
      If you would like to contact him, his address is jfrederich (at)
      gmail (dot) com.  Thanks Jens!
      </p>

      <h4>Gentoo Users</h4>

      <p>
      Portage generally has a relatively recent version of CGDB
      (<a href="http://www.gentoo-portage.com/dev-util/cgdb">dev-util/cgdb</a>),
      simply "emerge cgdb" to install it on your system.  As of this
      writing, 0.6.2 is unmasked, but 0.6.3 is masked with ~keyword.
      (They're both stable, Gentoo just doesn't keep up so well.)
      </p>

      <p>
      If you want to be totally current, you may want our
      <a href="files/cgdb-0.6.4.ebuild">latest Gentoo ebuild</a> (0.6.4)
      that you can use instead.  If you've never installed an ebuild before,
      check out the
      <a href="http://gentoo-wiki.com/HOWTO_Installing_3rd_Party_Ebuilds">Gentoo
      Wiki on 3rd party ebuilds</a>.  The main platforms (x86, amd64, ppc) are
      not masked, as we consider the tool stable, so you shouldn't have to
      unmask anything.
      </p>

      <h4>Installing CGDB manually</h4>

      <h5>Prerequisites</h5>

      <ol>
        <li>
        <a href="http://tiswww.case.edu/~chet/readline/rltop.html">readline</a>
        5.1 or greater.  An important design problem was fixed in version 5.1,
        any older version will not support CGDB.</li>

        <li>
        <a href="http://www.gnu.org/software/ncurses/ncurses.html">ncurses</a>
        (or curses, but it doesn't always look pretty), any recent version
        (i.e. 5+) will do.</li>
      </ol>

      <!-- Update this link every release -->
      <h5>Stable Release:</h5>
      <p>
      <a href="http://prdownloads.sourceforge.net/cgdb/cgdb-0.6.4.tar.gz?download">cgdb-0.6.4.tar.gz</a> -
      <a href="http://cgdb.svn.sourceforge.net/viewvc/cgdb/cgdb/trunk/ChangeLog?view=markup">ChangeLog</a> -
      <a href="http://cgdb.svn.sourceforge.net/viewvc/cgdb/cgdb/trunk/NEWS?view=markup">NEWS</a>
      </p>

      <h4>Subversion Access</h4>

      <p>
      The most recent changes to CGDB can be found by checking out
      a copy of the current
      <a href="http://subversion.tigris.org/">Subversion</a> tree.  You may
      want to try this to see if a bug is resolved before reporting the error.
      It should be noted that the svn tree is not guaranteed to be stable and
      is not tested as well as the latest stable release.
      </p>

      <p>
      To get a copy of the svn repository, use the following command:
      <code>
        svn co https://svn.sourceforge.net/svnroot/cgdb/cgdb/trunk cgdb
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
      <ul>
        <li>aclocal  (GNU automake)   1.9.6</li>
        <li>autoconf (GNU Autoconf)   2.59</li>
        <li>m4       (GNU m4)         1.4.4</li>
      </ul>
      </p>

      <p>
      Finally, if you decide to modify the tokenizer library or the config
      file module, you will need to have flex installed.  We have used flex
      2.5.4 to build cgdb.
      </p>

    </div>

<?
    include "footer.php";
?>
