<?
    define('PAGE_ID', 'error_page');
    include "header.php";
?>

    <h3>Page Not Found</h3>

    <div class="pagetext">

      <p>
        The page you tried to reach does not exist.
        <a href="./">Click here</a> to visit our home page, or use the
        navigation links at the top of this page to find a specific
        part of the site.
      </p>

      <p>
        If you were brought here by a link from another site, you may
        wish to notify them to update their links.  If you have this
        page bookmarked, you may wish to update your bookmarks.
      </p>

      <p>
        <small><i>
          HTTP Error 404<br />
          Generated on <?= date('l, F dS, Y h:i:s A') ?>
        </i></small>
      </p>
        
    </div>

<?
    include "footer.php";
?>
