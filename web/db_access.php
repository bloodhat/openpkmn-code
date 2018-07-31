<?php

function get_user_status($username, $connection)
{
  $query = sprintf("SELECT Status FROM openpkmn.Users ".
		   "WHERE Name = '%s'",
		   mysql_real_escape_string($username));
  
  $result = mysql_query($query, $connection);
  $row = mysql_fetch_assoc($result);
  
  return $row['Status'];
}

 
function get_user_wins($username, $connection)
{
  $query = sprintf("SELECT COUNT(*) AS Wins FROM openpkmn.Battles B, ".
		   "openpkmn.Users U WHERE U.Name = '%s' ".
		   "AND U.id = B.Winner_id",
		   mysql_real_escape_string($username));
  
  $result = mysql_query($query, $connection);
  $row = mysql_fetch_assoc($result);
  
  return $row["Wins"];
}
 
function get_user_losses($username, $connection)
{
	$query = sprintf("SELECT COUNT(*) AS Losses FROM openpkmn.Battles B, ".
			 "openpkmn.Users U WHERE U.Name = '%s' ".
			 "AND U.id = B.Loser_id",
			 mysql_real_escape_string($username));

	$result = mysql_query($query, $connection);
	$row = mysql_fetch_assoc($result);
	
	return $row["Losses"];
}

function set_user_server_status_logged_out($username, $connection)
{
  $query = sprintf("UPDATE openpkmn.Users U SET Status = '0' WHERE  U.Name = '%s'",
		   mysql_real_escape_string($username));

  $result = mysql_query($query, $connection);
  
  return $result;
}

?>