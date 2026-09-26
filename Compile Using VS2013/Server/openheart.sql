-- ##################################################################################################
-- ##------------------------------------------FILE START------------------------------------------##
-- ##################################################################################################


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
 SET NAMES utf8 ;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

-- Create and select the database.
--
-- The name below (`feralheart`) must match the first line of your
-- `db` config file. To use a different name, replace `feralheart`
-- in BOTH lines below.

CREATE DATABASE IF NOT EXISTS `feralheart` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `feralheart`;

--
-- Table structure for table `fh_appearance`
--

DROP TABLE IF EXISTS `fh_appearance`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_appearance` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `species` varchar(45) DEFAULT NULL,
  `mesh_0` tinyint(3) unsigned DEFAULT NULL,
  `mesh_1` tinyint(3) unsigned DEFAULT NULL,
  `mesh_2` tinyint(3) unsigned DEFAULT NULL,
  `mesh_3` tinyint(3) unsigned DEFAULT NULL,
  `mesh_4` tinyint(3) unsigned DEFAULT NULL,
  `mat_0` tinyint(3) unsigned DEFAULT NULL,
  `mat_1` tinyint(3) unsigned DEFAULT NULL,
  `mat_2` tinyint(3) unsigned DEFAULT NULL,
  `mat_3` tinyint(3) unsigned DEFAULT NULL,
  `scale_0` tinyint(4) DEFAULT NULL,
  `scale_1` tinyint(4) DEFAULT NULL,
  `scale_2` tinyint(4) DEFAULT NULL,
  `scale_3` tinyint(4) DEFAULT NULL,
  `scale_4` tinyint(4) DEFAULT NULL,
  `scale_5` tinyint(4) DEFAULT NULL,
  `scale_6` tinyint(4) DEFAULT NULL,
  `scale_7` tinyint(4) DEFAULT NULL,
  `scale_8` tinyint(4) DEFAULT NULL,
  `scale_9` tinyint(4) DEFAULT NULL,
  `colour_0_0` tinyint(3) unsigned DEFAULT NULL,
  `colour_0_1` tinyint(3) unsigned DEFAULT NULL,
  `colour_0_2` tinyint(3) unsigned DEFAULT NULL,
  `colour_1_0` tinyint(3) unsigned DEFAULT NULL,
  `colour_1_1` tinyint(3) unsigned DEFAULT NULL,
  `colour_1_2` tinyint(3) unsigned DEFAULT NULL,
  `colour_2_0` tinyint(3) unsigned DEFAULT NULL,
  `colour_2_1` tinyint(3) unsigned DEFAULT NULL,
  `colour_2_2` tinyint(3) unsigned DEFAULT NULL,
  `colour_3_0` tinyint(3) unsigned DEFAULT NULL,
  `colour_3_1` tinyint(3) unsigned DEFAULT NULL,
  `colour_3_2` tinyint(3) unsigned DEFAULT NULL,
  `colour_4_0` tinyint(3) unsigned DEFAULT NULL,
  `colour_4_1` tinyint(3) unsigned DEFAULT NULL,
  `colour_4_2` tinyint(3) unsigned DEFAULT NULL,
  `colour_5_0` tinyint(3) unsigned DEFAULT NULL,
  `colour_5_1` tinyint(3) unsigned DEFAULT NULL,
  `colour_5_2` tinyint(3) unsigned DEFAULT NULL,
  `colour_6_0` tinyint(3) unsigned DEFAULT NULL,
  `colour_6_1` tinyint(3) unsigned DEFAULT NULL,
  `colour_6_2` tinyint(3) unsigned DEFAULT NULL,
  `colour_7_0` tinyint(3) unsigned DEFAULT NULL,
  `colour_7_1` tinyint(3) unsigned DEFAULT NULL,
  `colour_7_2` tinyint(3) unsigned DEFAULT NULL,
  `colour_8_0` tinyint(3) unsigned DEFAULT NULL,
  `colour_8_1` tinyint(3) unsigned DEFAULT NULL,
  `colour_8_2` tinyint(3) unsigned DEFAULT NULL,
  `emote` tinyint(3) unsigned DEFAULT NULL,
  `preset` tinyint(3) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fh_appearance`
--

LOCK TABLES `fh_appearance` WRITE;
/*!40000 ALTER TABLE `fh_appearance` DISABLE KEYS */;
/*!40000 ALTER TABLE `fh_appearance` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fh_banip`
--

DROP TABLE IF EXISTS `fh_banip`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_banip` (
  `IP` varchar(45) DEFAULT NULL,
  `ref` int(10) unsigned DEFAULT NULL,
  `ban_end` datetime DEFAULT NULL,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fh_banip`
--

LOCK TABLES `fh_banip` WRITE;
/*!40000 ALTER TABLE `fh_banip` DISABLE KEYS */;
/*!40000 ALTER TABLE `fh_banip` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fh_banmac`
--

DROP TABLE IF EXISTS `fh_banmac`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_banmac` (
  `mac` varchar(45) DEFAULT NULL,
  `uid` int(10) unsigned DEFAULT NULL,
  `ban_end` datetime DEFAULT NULL,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fh_banmac`
--

LOCK TABLES `fh_banmac` WRITE;
/*!40000 ALTER TABLE `fh_banmac` DISABLE KEYS */;
/*!40000 ALTER TABLE `fh_banmac` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fh_block`
--

DROP TABLE IF EXISTS `fh_block`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_block` (
  `sender_id` int(10) unsigned DEFAULT NULL,
  `target_id` int(10) unsigned DEFAULT NULL,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fh_block`
--

LOCK TABLES `fh_block` WRITE;
/*!40000 ALTER TABLE `fh_block` DISABLE KEYS */;
/*!40000 ALTER TABLE `fh_block` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fh_characters`
--

DROP TABLE IF EXISTS `fh_characters`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_characters` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(10) unsigned DEFAULT NULL,
  `name` varchar(45) DEFAULT NULL,
  `map` varchar(45) DEFAULT NULL,
  `pos_x` int(11) DEFAULT NULL,
  `pos_y` int(11) DEFAULT NULL,
  `pos_z` int(11) DEFAULT NULL,
  `home_map` varchar(45) DEFAULT NULL,
  `home_x` int(11) DEFAULT NULL,
  `home_y` int(11) DEFAULT NULL,
  `home_z` int(11) DEFAULT NULL,
  `group_id` int(10) unsigned DEFAULT NULL,
  `group_title` varchar(45) DEFAULT NULL,
  `group_rank` int(11) DEFAULT NULL,
  `bio` text,
  `token` varchar(11) DEFAULT NULL,
  `token_time` datetime DEFAULT NULL,
  `is_online` tinyint(4) DEFAULT NULL,
  `party_id` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fh_characters`
--

LOCK TABLES `fh_characters` WRITE;
/*!40000 ALTER TABLE `fh_characters` DISABLE KEYS */;
/*!40000 ALTER TABLE `fh_characters` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fh_friend`
--

DROP TABLE IF EXISTS `fh_friend`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_friend` (
  `sender_id` int(10) unsigned DEFAULT NULL,
  `target_id` int(10) unsigned DEFAULT NULL,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fh_friend`
--

LOCK TABLES `fh_friend` WRITE;
/*!40000 ALTER TABLE `fh_friend` DISABLE KEYS */;
/*!40000 ALTER TABLE `fh_friend` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fh_friend_request`
--

DROP TABLE IF EXISTS `fh_friend_request`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_friend_request` (
  `sender_id` int(10) unsigned DEFAULT NULL,
  `target_id` int(10) unsigned DEFAULT NULL,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fh_friend_request`
--

LOCK TABLES `fh_friend_request` WRITE;
/*!40000 ALTER TABLE `fh_friend_request` DISABLE KEYS */;
/*!40000 ALTER TABLE `fh_friend_request` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fh_group_invite`
--

DROP TABLE IF EXISTS `fh_group_invite`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_group_invite` (
  `group_id` int(10) unsigned DEFAULT NULL,
  `char_id` int(10) unsigned DEFAULT NULL,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fh_group_invite`
--

LOCK TABLES `fh_group_invite` WRITE;
/*!40000 ALTER TABLE `fh_group_invite` DISABLE KEYS */;
/*!40000 ALTER TABLE `fh_group_invite` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fh_group_request`
--

DROP TABLE IF EXISTS `fh_group_request`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_group_request` (
  `char_id` int(10) unsigned DEFAULT NULL,
  `group_id` int(10) unsigned DEFAULT NULL,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fh_group_request`
--

LOCK TABLES `fh_group_request` WRITE;
/*!40000 ALTER TABLE `fh_group_request` DISABLE KEYS */;
/*!40000 ALTER TABLE `fh_group_request` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fh_groups`
--

DROP TABLE IF EXISTS `fh_groups`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_groups` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(45) DEFAULT NULL,
  `leader_id` int(10) unsigned DEFAULT NULL,
  `bio` varchar(45) DEFAULT NULL,
  `private` tinyint(4) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fh_groups`
--

LOCK TABLES `fh_groups` WRITE;
/*!40000 ALTER TABLE `fh_groups` DISABLE KEYS */;
INSERT INTO `fh_groups` VALUES (1,'admins',0,NULL,NULL);
/*!40000 ALTER TABLE `fh_groups` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fh_log`
--

DROP TABLE IF EXISTS `fh_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_log` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `DateTime` datetime DEFAULT NULL,
  `uid` int(10) unsigned DEFAULT NULL,
  `sf` varchar(45) DEFAULT NULL,
  `IP` varchar(45) DEFAULT NULL,
  `MAC` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=128 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fh_log`
--

LOCK TABLES `fh_log` WRITE;
/*!40000 ALTER TABLE `fh_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `fh_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fh_modgroup`
--

DROP TABLE IF EXISTS `fh_modgroup`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_modgroup` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `ghost` tinyint(4) DEFAULT NULL,
  `kick` tinyint(4) DEFAULT NULL,
  `ban` tinyint(4) DEFAULT NULL,
  `unban` tinyint(4) DEFAULT NULL,
  `godspeak` tinyint(4) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fh_modgroup`
--

LOCK TABLES `fh_modgroup` WRITE;
/*!40000 ALTER TABLE `fh_modgroup` DISABLE KEYS */;
/*!40000 ALTER TABLE `fh_modgroup` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fh_reports`
--

DROP TABLE IF EXISTS `fh_reports`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_reports` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `reporter_id` int(10) unsigned DEFAULT NULL,
  `target_id` int(10) unsigned DEFAULT NULL,
  `message` text,
  `DateTime` datetime DEFAULT NULL,
  `status` tinyint(4) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fh_reports`
--

LOCK TABLES `fh_reports` WRITE;
/*!40000 ALTER TABLE `fh_reports` DISABLE KEYS */;
/*!40000 ALTER TABLE `fh_reports` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fh_users`
--

DROP TABLE IF EXISTS `fh_users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_users` (
  `uid` int(10) unsigned DEFAULT NULL,
  `mod_group` int(11) DEFAULT NULL,
  `ban_end` datetime DEFAULT NULL,
  `last` datetime DEFAULT CURRENT_TIMESTAMP,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fh_users`
--

LOCK TABLES `fh_users` WRITE;
/*!40000 ALTER TABLE `fh_users` DISABLE KEYS */;
/*!40000 ALTER TABLE `fh_users` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `jos_users`
--

DROP TABLE IF EXISTS `jos_users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
 SET character_set_client = utf8 ;
CREATE TABLE `jos_users` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `password` varchar(100) DEFAULT NULL,
  `block` tinyint(4) DEFAULT NULL,
  `username` varchar(150) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `jos_users`
--

LOCK TABLES `jos_users` WRITE;
/*!40000 ALTER TABLE `jos_users` DISABLE KEYS */;
/*!40000 ALTER TABLE `jos_users` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;


-- ##################################################################################################
-- ##-------------------------------------------FILE END-------------------------------------------##
-- ##################################################################################################
-- ##-------------------------------------------MODIFIED-------------------------------------------##
-- ## DATE: 9/25/2026                                                                              ##
-- ## TIME: 7:32 PM (UTC-5:00)                                                                     ##
-- ## BY: radioactivedark                                                                          ##
-- ##################################################################################################