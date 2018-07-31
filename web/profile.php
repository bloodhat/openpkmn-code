<?php
require("db_config.php");
require("db_access.php");
require("strings.php");

if(!($connection = mysql_connect($db_server, $db_user, $db_pass)))
  {
    die("could not connect the database");
  }

if (!isset($_COOKIE["username"]))
  {
    header('Location:index.html');
  }
else
  {
    $username = $_COOKIE["username"];
?>
      <html><body>
      <h1>Welcome <? echo($username." (".
			  get_user_wins($username, $connection).
			  "-".get_user_losses($username, $connection).")"); ?>
	 </h1>
      Your current status on the battle server:
      <? echo($status_strings[get_user_status($username, $connection)]); ?> |  
     <a href="client.jnlp">Log into the battle server</a>
	(<a href="server_logout.php">I can't log in!</a>)<br>
     <h2>Recent Battles</h2>
</body></html>

<?php
  }
?>
