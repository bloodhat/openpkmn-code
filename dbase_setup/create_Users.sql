CREATE TABLE `Users` (
  `ID` int(11) NOT NULL auto_increment,
  `Name` varchar(32) character set latin1 NOT NULL,
  `Status` int(11) NOT NULL,
  `Ruleset` int(11) default 0 NOT NULL,
  `Key_data` char(32) default NULL,
  `Key_type` int(11) default NULL,
  `selected_team` int(11) default NULL,
  `selected_starter` int(11) default NULL,
  PRIMARY KEY  (`ID`),
  UNIQUE KEY `Name` (`Name`),
  KEY `Users_team` (`selected_team`),
  KEY `Users_starter` (`selected_starter`),
  CONSTRAINT `Users_team` FOREIGN KEY (`selected_team`) REFERENCES `Teams` (`ID`),
  CONSTRAINT `Users_starter` FOREIGN KEY (`selected_starter`) REFERENCES `PKMN` (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=30 DEFAULT CHARSET=ascii