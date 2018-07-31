CREATE TABLE `Teams` (
  `ID` int(11) NOT NULL auto_increment,
  `owner_id` int(11) NOT NULL,
  `ruleset` int(11),
  `visible` tinyint(1) NOT NULL,
  PRIMARY KEY  (`ID`),
  KEY `team_owner` (`owner_id`),
  CONSTRAINT `team_owner` FOREIGN KEY (`owner_id`) REFERENCES `Users` (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=130 DEFAULT CHARSET=ascii