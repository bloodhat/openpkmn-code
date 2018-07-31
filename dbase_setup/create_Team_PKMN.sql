CREATE TABLE `Team_PKMN` (
  `ID` int(11) NOT NULL auto_increment,
  `team_id` int(11) default NULL,
  `pkmn_id` int(11) NOT NULL,
  `rost_num` int(11) NOT NULL,
  PRIMARY KEY  (`ID`),
  KEY `tp_team` (`team_id`),
  KEY `tp_pkmn` (`pkmn_id`),
  CONSTRAINT `tp_pkmn` FOREIGN KEY (`pkmn_id`) REFERENCES `PKMN` (`ID`),
  CONSTRAINT `tp_team` FOREIGN KEY (`team_id`) REFERENCES `Teams` (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=279 DEFAULT CHARSET=latin1 COMMENT='Many-Many relationship between PKMN and teams'