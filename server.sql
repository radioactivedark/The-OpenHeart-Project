CREATE DATABASE IF NOT EXISTS server;
USE server;
DROP TABLE IF EXISTS `fh_appearance`;
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
LOCK TABLES `fh_appearance` WRITE;
UNLOCK TABLES;
DROP TABLE IF EXISTS `fh_banip`;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_banip` (
  `IP` varchar(45) DEFAULT NULL,
  `ref` int(10) unsigned DEFAULT NULL,
  `ban_end` datetime DEFAULT NULL,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;


LOCK TABLES `fh_banip` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `fh_banmac`;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_banmac` (
  `mac` varchar(45) DEFAULT NULL,
  `uid` int(10) unsigned DEFAULT NULL,
  `ban_end` datetime DEFAULT NULL,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;


LOCK TABLES `fh_banmac` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `fh_block`;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_block` (
  `sender_id` int(10) unsigned DEFAULT NULL,
  `target_id` int(10) unsigned DEFAULT NULL,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;


LOCK TABLES `fh_block` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `fh_characters`;
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


LOCK TABLES `fh_characters` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `fh_friend`;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_friend` (
  `sender_id` int(10) unsigned DEFAULT NULL,
  `target_id` int(10) unsigned DEFAULT NULL,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;


LOCK TABLES `fh_friend` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `fh_friend_request`;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_friend_request` (
  `sender_id` int(10) unsigned DEFAULT NULL,
  `target_id` int(10) unsigned DEFAULT NULL,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;


LOCK TABLES `fh_friend_request` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `fh_group_invite`;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_group_invite` (
  `group_id` int(10) unsigned DEFAULT NULL,
  `char_id` int(10) unsigned DEFAULT NULL,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;


LOCK TABLES `fh_group_invite` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `fh_group_request`;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_group_request` (
  `char_id` int(10) unsigned DEFAULT NULL,
  `group_id` int(10) unsigned DEFAULT NULL,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;


LOCK TABLES `fh_group_request` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `fh_groups`;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_groups` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(45) DEFAULT NULL,
  `leader_id` int(10) unsigned DEFAULT NULL,
  `bio` varchar(45) DEFAULT NULL,
  `private` tinyint(4) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;


LOCK TABLES `fh_groups` WRITE;
INSERT INTO `fh_groups` VALUES (1,'admins',0,NULL,NULL);
UNLOCK TABLES;


DROP TABLE IF EXISTS `fh_log`;
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


LOCK TABLES `fh_log` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `fh_modgroup`;
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


LOCK TABLES `fh_modgroup` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `fh_reports`;
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


LOCK TABLES `fh_reports` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `fh_users`;
 SET character_set_client = utf8 ;
CREATE TABLE `fh_users` (
  `uid` int(10) unsigned DEFAULT NULL,
  `mod_group` int(11) DEFAULT NULL,
  `ban_end` datetime DEFAULT NULL,
  `last` datetime DEFAULT CURRENT_TIMESTAMP,
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;


LOCK TABLES `fh_users` WRITE;
UNLOCK TABLES;


DROP TABLE IF EXISTS `jos_users`;
 SET character_set_client = utf8 ;
CREATE TABLE `jos_users` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `password` varchar(100) DEFAULT NULL,
  `block` tinyint(4) DEFAULT NULL,
  `username` varchar(150) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;


LOCK TABLES `jos_users` WRITE;
UNLOCK TABLES;