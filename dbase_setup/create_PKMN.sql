CREATE TABLE `PKMN` (
  `ID` int(11) NOT NULL auto_increment,
  `Species` int(11) NOT NULL,
  `Owner_ID` int(11) NOT NULL,
  `nickname` varchar(32) NOT NULL,
  `level` int(11) NOT NULL,
  `max_hp` int(11) NOT NULL,
  `attack` int(11) NOT NULL,
  `defense` int(11) NOT NULL,
  `speed` int(11) NOT NULL,
  `special` int(11) NOT NULL,
  `ruleset` int(11) NOT NULL,
  `visible` tinyint(1) NOT NULL,
  PRIMARY KEY  (`ID`),
  KEY `Pkmn_Owner` (`Owner_ID`),
  CONSTRAINT `Pkmn_Owner` FOREIGN KEY (`Owner_ID`) REFERENCES `Users` (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=78 DEFAULT CHARSET=ascii