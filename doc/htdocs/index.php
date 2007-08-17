<?
    define('PAGE_ID', 'home_page');
    include "header.php";
?>

    <h3>Description</h3>

    <div class="pagetext">
      <p>
        CGDB is a curses-based (i.e. terminal-based) interface to the GNU
        Debugger (GDB).  Its goal is to be lightweight and responsive; not
        encumbered with unnecessary features.
      </p>
    
      <p>
        The primary feature of CGDB is the constant presence of a source
        display (<a href="screenshots.php">screenshots</a>), updated as the
        program executes, to help keep you focused while debugging.
        The interface is inspired by the classic Unix text editor, vi.  Those
        familiar with vi (or vim) should feel right at home using CGDB.
      </p>

      <p>
        <a href="download.php">Download</a> CGDB today!
      </p>
    </div>

    <h3>Recent News</h3>
    
    <div class="news_entry">
      <div class="news_date">
        August 17th, 2007
      </div>

      <div class="news_text">
        <div>
          The new CGDB web site has been deployed!  We hope you like it.
          The page has been tested in Internet Explorer, Firefox, and
          Opera, but if you encounter problems, please report them to
          mike(at)subfocal(dot)net.  Thanks!
        </div>
      </div>
    </div>

    <div class="news_entry">
      <div class="news_date">
        April 28, 2007
      </div>

      <div class="news_text">
        <div>
          CGDB v0.6.4 was released today, see the download page to try it out.
          This release is mostly important because it implements the vim 'map'
          command. The functionality is not entirely polished off, so mappings
          like 'map a a' will cause CGDB to loop infinitely. The next release
          of CGDB will most likely provide the finishing touches to this
          functionality, along with the documentation in the manual.
        </div>

        <div>
          A few interesting features that have been added in this release are:
          <ul>
            <li>
              Added a new highlighting group, SelectedLineNr. This is the
              line that the cursor is on.
            </li>
            <li>CGDB's build system should compile with the -j option</li>
            <li>CGDB shuts down now when it receives ctrl-d, just like GDB</li>
            <li>
              CGDB no longer displays (tgdb) at the gdb prompt. It now
              displays (gdb)
            </li>
          </ul>
        </div>

        <div>
          Finally, this release also has some pretty important bug fixes.
          There was a bug in the communication between CGDB and gdb that
          would cause it to sometimes simply work improperly. As always
          see the NEWS file for a complete list of changes since the last
          release.
        </div>

        <div>
          Please also note that the escdelay option has been removed. A
          user should now use the timeout, ttimeout, timeoutlen and
          ttimeoutlen options. They are described in the manual. 
        </div>
      </div>
    </div>

    <div class="news_entry">
      <div class="news_date">
        June 3, 2006
      </div>

      <div class="news_text">
        <div>
          CGDB v0.6.3 was released today, see the
          <a href="download.php">download page</a> to try it out. This release
          changes the way CGDB starts GDB. CGDB now puts GDB on a pty instead
          of a pipe. This allows GDB to ask several questions to the user that
          would not be asked on the pipe. This makes using CGDB even more
          similar to just using GDB. There were some autoconf configure
          problems that were reported involving readline. These have also been
          fixed. Several other small bugs have been fixed.
        </div>
      
        <div>
          At last! CGDB has finally converted to the subversion version
          control system.  Although this doesn't effect users that much,
          maybe I can lure more contributers into developing CGDB. :)
        </div>
      </div>
    </div>

    <div class="news_entry">
      <div class="news_date">
        April 9, 2006
      </div>

      <div class="news_text">
        <div>
          CGDB v0.6.2 was released today, see the
          <a href="download.php">download page</a> to try it out. This
          release get's CGDB working on Solaris again.  Also, it fixes
          some autotool problems that could have cause corruption in the
          GDB window. It also fixes a regression from cgdb-0.6.1, the message
          &quot;CGDB had unexpected results, ...&quot; will no longer be
          displayed when shutting CGDB down if you set a watch point while
          debugging.
        </div>

        <div>
          Enjoy.
        </div>
      </div>
    </div>

    <div id="sourceforge_logo">
      <a href="http://sourceforge.net"><img src="http://sflogo.sourceforge.net/sflogo.php?group_id=72581&amp;type=3" width="125" height="37" border="0" alt="SourceForge.net Logo" /></a>
    </div>

<?
    include "footer.php";
?>
