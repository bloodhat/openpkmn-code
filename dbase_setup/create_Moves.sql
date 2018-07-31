CREATE TABLE `Moves` (
  `ID` int(11) NOT NULL auto_increment,
  `Move_number` int(11) NOT NULL,
  `Max_pp` int(11) NOT NULL,
  `Pkmn_id` int(11) NOT NULL,
  `Owner_id` int(11) NOT NULL,
  PRIMARY KEY  (`ID`),
  KEY `Move_Owner` (`Owner_id`),
  KEY `Move_Pkmn` (`Pkmn_id`),
  CONSTRAINT `Move_Owner` FOREIGN KEY (`Owner_id`) REFERENCES `Users` (`ID`),
  CONSTRAINT `Move_Pkmn` FOREIGN KEY (`Pkmn_id`) REFERENCES `PKMN` (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=253 DEFAULT CHARSET=ascii