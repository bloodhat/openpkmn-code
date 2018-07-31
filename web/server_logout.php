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
    set_user_server_status_logged_out($username, $connection);
    header('Location:profile.php');
  }
?>