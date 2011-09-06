<?
    define('PAGE_ID', 'home_page');
    include "header.php";
?>

    <h3>Description</h3>

    <div id="cgdb_guy">
      <a href="images/cgdb_guy.png"><img src="images/cgdb_guy_tn.png"
        width="60" height="60" alt="CGDB Guy" /></a>
    </div>

    <div class="pagetext">
      <p>
        CGDB is a curses (terminal-based) interface to the GNU
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

      <div id="features">
      <h4>Features</h4>
      
      <ul>
        <li>Syntax-highlighted source window</li>
        <li>Visual breakpoint setting</li>
        <li>Keyboard shortcuts for common functions</li>
        <li>Searching source window (using regexp)</li>
        <li>Scrollable gdb history of entire session</li>
        <li>Tab completion</li>
        <li>Key mappings (macros)</li>
      </ul>
      </div>

      <div id="sponsor">
      <p style="color: #888; font-size: smaller;">
        &mdash; Sponsor &mdash;
      </p>
      <p>
        <a href="http://www.vigilantsw.com/"><b>Vigilant Software</b></a>
      </p>
      <p>
        <small>
          Advanced static code analysis for C and C++.
          Try Vigilant Sentry and start finding bugs today.
        </small>
      </p>
      </p>
      </div>

      <div style="clear:both"></div>

    </div>

    <h3>Recent News</h3>

    <div class="news_entry">
      <div class="news_date">
        September 6, 2011
      </div>

      <div class="news_text">
        <p>
          CGDB v0.6.6 was released today, see the download page to try it out.
        </p>
        <p>
          This release is primarily a bug-fix release and contains no major
          new features.  It fixes a segfault on some 64-bit systems, a compile
          error on Cygwin 1.7, and some search issues in the file dialog.
        </p>
        <p>
          We encourage anyone running cgdb-0.6.5 or older to update.
        </p>
      </div>
    </div>

    <div class="news_entry">
      <div class="news_date">
        January 3rd, 2010
      </div>

      <div class="news_text">
        <p>
          CGDB v0.6.5 was released today, see the download page to try it out.
        </p>

        <p>
          One change that will likely get a lot of attention is that the
          CGDB shortcut mode has been removed from cgdb. If you were using
          it, you can get the same functionality by adding these commands
          into your .cgdbrc,
          <ul>
            <li>map r :run&lt;CR&gt;</li>
            <li>map c :continue&lt;CR&gt;</li>
            <li>map f :finish&lt;CR&gt;</li>
            <li>map n :next&lt;CR&gt;</li>
            <li>map s :step&lt;CR&gt;</li>
          </ul>
        </p>

        <p>
          A few interesting features that have been added in this release are:
        </p>

        <ul>
          <li>The autosourcereload option is turned on by default</li>
          <li>CGDB should successfully build on Mac OS X 10.5</li>
          <li>Add support for :up and :down in the cgdb status bar</li>
          <li>Add support for GNAT annotations (see NOTES)</li>
          <li>Made searching and :commands a little more vi-like (see NOTES)</li>
        </ul>

        <p>
          This release also has serveral bug fixes and features that can
          be seen in the NEWS file. As always, happy CGDB'ing.
        </p>
      </div>
    </div>

    
    <div class="news_entry">
      <div class="news_date">
        August 17th, 2007
      </div>

      <div class="news_text">
        <p>
          The new CGDB web site has been deployed!  We hope you like it.
          The page has been tested in Internet Explorer, Firefox, and
          Opera, but if you encounter problems, please report them to
          mike(at)subfocal(dot)net.  Thanks!
        </p>
      </div>
    </div>

    <div class="news_entry">
      <div class="news_date">
        April 28, 2007
      </div>

      <div class="news_text">
        <p>
          CGDB v0.6.4 was released today, see the download page to try it out.
          This release is mostly important because it implements the vim 'map'
          command. The functionality is not entirely polished off, so mappings
          like 'map a a' will cause CGDB to loop infinitely. The next release
          of CGDB will most likely provide the finishing touches to this
          functionality, along with the documentation in the manual.
        </p>

        <p>
          A few interesting features that have been added in this release are:
        </p>

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

        <p>
          Finally, this release also has some pretty important bug fixes.
          There was a bug in the communication between CGDB and gdb that
          would cause it to sometimes simply work improperly. As always
          see the NEWS file for a complete list of changes since the last
          release.
        </p>

        <p>
          Please also note that the escdelay option has been removed. A
          user should now use the timeout, ttimeout, timeoutlen and
          ttimeoutlen options. They are described in the manual. 
        </p>
      </div>
    </div>

    <div class="news_entry">
      <div class="news_date">
        June 3, 2006
      </div>

      <div class="news_text">
        <p>
          CGDB v0.6.3 was released today, see the
          <a href="download.php">download page</a> to try it out. This release
          changes the way CGDB starts GDB. CGDB now puts GDB on a pty instead
          of a pipe. This allows GDB to ask several questions to the user that
          would not be asked on the pipe. This makes using CGDB even more
          similar to just using GDB. There were some autoconf configure
          problems that were reported involving readline. These have also been
          fixed. Several other small bugs have been fixed.
        </p>
      
        <p>
          At last! CGDB has finally converted to the subversion version
          control system.  Although this doesn't effect users that much,
          maybe I can lure more contributers into developing CGDB. :)
        </p>
      </div>
    </div>

    <div class="news_entry">
      <div class="news_date">
        April 9, 2006
      </div>

      <div class="news_text">
        <p>
          CGDB v0.6.2 was released today, see the
          <a href="download.php">download page</a> to try it out. This
          release get's CGDB working on Solaris again.  Also, it fixes
          some autotool problems that could have cause corruption in the
          GDB window. It also fixes a regression from cgdb-0.6.1, the message
          &quot;CGDB had unexpected results, ...&quot; will no longer be
          displayed when shutting CGDB down if you set a watch point while
          debugging.
        </p>

        <p>
          Enjoy.
        </p>
      </div>
    </div>

    <div id="sourceforge_logo">
      <a href="http://sourceforge.net"><img src="http://sflogo.sourceforge.net/sflogo.php?group_id=72581&amp;type=3" width="125" height="37" alt="SourceForge.net Logo" /></a>
    </div>

    <div id="spacer">
    </div>

<?
    include "footer.php";
?>
