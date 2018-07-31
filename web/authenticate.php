<?php
require('db_config.php');

function setup_session($username, $password)
{
  setcookie("username", $username, time()+3600);
  setcookie("password", $password, time()+3600);
  $_COOKIE["username"] = $username;
  $_COOKIE["password"] = $password;
}

if (isset($_POST["deletecookie"]))
  {
    setcookie("username", "", time()-3600);
    unset($_COOKIE["username"]);
    unset($_COOKIE["password"]);
  }
else if(isset($_COOKIE["username"]))
  {
     header('Location:profile.php');
  }
else if((@$_POST["username"] != "") && (@$_POST["password"] != ""))
  {
    $connection = mysql_connect($db_server, $db_user, $db_pass);
    $username = $_POST["username"];
    $pass  = $_POST["password"];
    if (!$connection)
      {
	die('Could not connect: '.mysql_error());
      }

    if($_POST["register"])
      {
	$query = sprintf("INSERT INTO openpkmn.Users".
			 "(Name, Status, Key_data, Key_type) ".
			 "VALUES('%s', '0', MD5('%s'), '1')",
			 mysql_real_escape_string($username),
			 mysql_real_escape_string($pass));
      
	$result = mysql_query($query);

	
	if (!$result)
	  {
	    header('Location:index.html');
	  }
	else
	  {
	    setup_session($username, $pass);
	    header('Location:profile.php');
	  }
      }
    else
      {
	$query = sprintf("SELECT Key_data FROM openpkmn.Users ".
			 "WHERE Name = '%s'",
			 mysql_real_escape_string($username));

	$result = mysql_query($query);
	$row = mysql_fetch_assoc($result);
	
	if($row['Key_data'] == md5($pass))
	  {
	    setup_session($username, $pass);
	    header('Location:profile.php');
	  }
	else
	  {
	    header('Location:index.html');
	  }
      }
    mysql_close($connection);
  }
 else
   {
     header('Location:index.html');
   }
 
?>
