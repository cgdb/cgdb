<?
    define('PAGE_ID', "screenshots_page");

    include "header.php";
?>

    <h3>Screenshots</h3>

    <div class="pagetext">
      <div>
        <img id="shot" src="images/ss_welcome.png" width="508" height="498"
             alt="Current screenshot" />
      </div>

      <div>
        Choose a screenshot below:
      </div>

      <div id="ss_thumbs">
        <div class="ss_thumb">
          <a onclick="javascript:document.getElementById('shot').
              src='images/ss_welcome.png'">
          <img src="images/ss_welcome.png" height="120" width="118"
               alt="Welcome screen" /><br/>
          Welcome</a>
        </div>

        <div class="ss_thumb">
          <a onclick="javascript:document.getElementById('shot').
              src='images/ss_debugging.png'">
          <img src="images/ss_debugging.png" height="120" width="118"
               alt="Debugging view" /><br />
          Debugging</a>
        </div>

        <div class="ss_thumb">
          <a onclick="javascript:document.getElementById('shot').
              src='images/ss_filedlg.png'">
          <img src="images/ss_filedlg.png" height="120" width="118"
               alt="File dialog" /><br />
          File Dialog</a>
        </div>
      </div>

      <div id="spacer">
      </div>

    </div>

<?
    include "footer.php";
?>
