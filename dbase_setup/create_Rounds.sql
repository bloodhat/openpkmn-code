CREATE TABLE `Rounds` (
  `ID` int(11) NOT NULL auto_increment,
  `Round_number` int(11) NOT NULL,
  `battle_id` int(11) NOT NULL,
  `round_end_state` int(11) NOT NULL,
  PRIMARY KEY  (`ID`),
  KEY `Battle` (`Round_number`),
  KEY `Round_Battle` (`battle_id`),
  CONSTRAINT `Round_Battle` FOREIGN KEY (`battle_id`) REFERENCES `Battles` (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=145 DEFAULT CHARSET=ascii