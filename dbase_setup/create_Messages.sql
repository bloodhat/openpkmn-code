/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

DROP TABLE IF EXISTS `Messages`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Messages` (
  `id` int(11) NOT NULL,
  `text` varchar(256) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
LOCK TABLES `Messages` WRITE;
/*!40000 ALTER TABLE `Messages` DISABLE KEYS */;
INSERT INTO `Messages` VALUES (0 , '@u sent out @p!'),
(1 , '@p used @m!'),
(2 , '@p\'s attack missed!'),
(3 , 'But it failed!'),
(4 , 'It\'s not very effective...'),
(5 , 'It\'s super effective!'),
(6 , 'No effect!'),
(7 , '@p flew up high!'),
(8 , '@p dug a hole'),
(9 , '@p lowered its head!'),
(10 , 'Critical hit!'),
(11 , 'Hit @i times!'),
(12 , '@p\'s building energy!'),
(13 , '@p\'s substitute broke!'),
(14 , '@p\'s rage is building'),
(15 , '@p\'s attack rose!'),
(16 , '@p\'s attack greatly rose!'),
(17 , '@p\'s defense rose!'),
(18 , '@p\'s defense greatly rose!'),
(19 , '@p\'s speed rose!'),
(20 , '@p\'s speed greatly rose!'),
(21 , '@p\'s special rose!'),
(22 , '@p\'s special greatly rose!'),
(23 , '@p\'s evade rose!'),
(24 , '@p\'s getting pumped!'),
(25 , '@p\'s attack fell!'),
(26 , '@p\'s attack greatly fell!'),
(27 , '@p\'s defense fell!'),
(28 , '@p\'s defense greatly fell!'),
(29 , '@p\'s speed fell!'),
(30 , '@p\'s speed greatly fell!'),
(31 , '@p\'s special fell!'),
(32 , '@p\'s special greatly fell!'),
(33 , '@p\'s accuracy fell!'),
(34 , '@p\'s hurt by the burn!'),
(35 , '@p\'s hurt by poison!'),
(36 , 'Leech seed saps @p!'),
(37 , '@p fell asleep!'),
(38 , '@p was paralyzed!'),
(39 , '@p was burned!'),
(40 , '@p was poisoned!'),
(41 , '@p was badly poisoned!'),
(42 , '@p was frozen!'),
(43 , '@p became confused!'),
(44 , '@p was seeded!'),
(45 , 'All status changes were eliminated!'),
(46 , '@p\'s protected against special attacks!'),
(47 , '@p gained armor!'),
(48 , '@p\'s shrouded in mist!'),
(49 , '@p transformed into @o!'),
(50 , 'Converted type to @o\'s'),
(51 , '@p learned @m!'),
(52 , '@p regained health!'),
(53 , '@p fell asleep and became healthy!'),
(54 , 'It created a substitute!'),
(55 , '@p woke up!'),
(56 , '@p is fast asleep!'),
(57 , '@p is frozen solid!'),
(58 , '@p\'s fully paralyzed!'),
(59 , 'Fire defrosted @p!'),
(60 , '@p flinched!'),
(61 , '@p\'s @m was disabled!'),
(62 , '@p is confused!'),
(63 , 'It hurt itslef in its confusion!'),
(64 , '@p must recharge!'),
(65 , '@p can\'t move!'),
(66 , '@p\'s thrashing about!'),
(67 , '@p\'s hit by recoil!'),
(68 , '@p\'s confused no more!'),
(69 , '@p\'s disabled no more!'),
(70 , '@p unleashed energy!'),
(71 , '@p\'s attack rose!'),
(72 , '@p fainted!'),
(73 , '@p sucked HP from @o'),
(74 , '@p\'s attack continues!'),
(75 , '@u won!'),
(76 , '@u lost...'),
(77 , 'Tie...'),
(78 , '@u ran away...'),
(79 , 'Damage random number is @i'),
(80 , ''),
(81 , '@i damage'),
(82 , '@p is at @i HP'),
(83 , '@p\'s substitute is at @i HP'),
(84 , '@m is disabled!'),
(85 , '@p made a whirlwind!'),
(86 , '@p is glowing!'),
(87 , '@p is gathering sunlight!');
/*!40000 ALTER TABLE `Messages` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
