CREATE TABLE `Events` (
  `ID` int(11) NOT NULL auto_increment,
  `Type` int(11) NOT NULL,
  `Turn_ID` int(11) NOT NULL,
  `Details` int(11) NOT NULL,
  `Subject_id` int(11) NOT NULL,
  `Subject_pkmn_id` int(11) NOT NULL,
  `Object_id` int(11) NOT NULL,
  `Object_pkmn_id` int(11) NOT NULL,
  PRIMARY KEY  (`ID`),
  KEY `Turn` (`Turn_ID`),
  CONSTRAINT `Turn` FOREIGN KEY (`Turn_ID`) REFERENCES `Turns` (`ID`),
  CONSTRAINT `Subject` FOREIGN KEY (`Subject_id`) REFERENCES `Users` (`ID`),
  CONSTRAINT `Subject_pkmn` FOREIGN KEY (`Subject_pkmn_id`) REFERENCES `PKMN` (`ID`),
  CONSTRAINT `Object` FOREIGN KEY (`Object_id`) REFERENCES `Users` (`ID`),
  CONSTRAINT `Object_pkmn` FOREIGN KEY (`Object_pkmn_id`) REFERENCES `PKMN` (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=358 DEFAULT CHARSET=ascii ROW_FORMAT=FIXED