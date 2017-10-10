-- MySQL dump 10.16  Distrib 10.1.22-MariaDB, for Linux (x86_64)
--
-- Host: mariadb    Database: video
-- ------------------------------------------------------
-- Server version	10.1.19-MariaDB

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

--
-- Table structure for table `clip`
--

DROP TABLE IF EXISTS `clip`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `clip` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `video_id` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `uploaded_at` timestamp NULL DEFAULT NULL,
  `file_path` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `file_exists` tinyint(1) NOT NULL DEFAULT '0',
  `entity_type` varchar(100) COLLATE utf8_unicode_ci NOT NULL,
  `entity_id` varchar(100) COLLATE utf8_unicode_ci NOT NULL,
  `stream_id` varchar(100) COLLATE utf8_unicode_ci NOT NULL,
  `stream_key` varchar(100) COLLATE utf8_unicode_ci NOT NULL,
  `status` varchar(100) COLLATE utf8_unicode_ci NOT NULL,
  `title` longtext COLLATE utf8_unicode_ci NOT NULL,
  `started` int(11) NOT NULL,
  `duration` int(11) NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `clip`
--

LOCK TABLES `clip` WRITE;
/*!40000 ALTER TABLE `clip` DISABLE KEYS */;
/*!40000 ALTER TABLE `clip` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `deferred_task`
--

DROP TABLE IF EXISTS `deferred_task`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `deferred_task` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `task_key` varchar(256) COLLATE utf8_unicode_ci NOT NULL,
  `time_to_create` timestamp NULL DEFAULT NULL,
  `task_type` varchar(256) COLLATE utf8_unicode_ci NOT NULL,
  `task_params` text COLLATE utf8_unicode_ci NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `deferred_task`
--

LOCK TABLES `deferred_task` WRITE;
/*!40000 ALTER TABLE `deferred_task` DISABLE KEYS */;
/*!40000 ALTER TABLE `deferred_task` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `file_informations`
--

DROP TABLE IF EXISTS `file_informations`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `file_informations` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `status` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `act_id` int(10) unsigned DEFAULT NULL,
  `scene_id` int(10) unsigned DEFAULT NULL,
  `file_path` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `created_at` bigint(20) DEFAULT NULL,
  `finished_at` bigint(20) DEFAULT NULL,
  `file_size` int(11) DEFAULT NULL,
  `type` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `file_informations`
--

LOCK TABLES `file_informations` WRITE;
/*!40000 ALTER TABLE `file_informations` DISABLE KEYS */;
INSERT INTO `file_informations` VALUES (1,'',0,0,'fragments/3/10/34/20171003103438.wav',1507026878,0,100000,'chunk'),(2,NULL,1,5,'1.wav',10,20,NULL,'chunk'),(3,NULL,1,5,'2.wav',20,30,NULL,'chunk'),(4,NULL,1,5,'3.wav',30,40,NULL,'chunk'),(5,NULL,1,5,'4.wav',40,50,NULL,'chunk'),(6,NULL,1,5,'5.wav',50,60,NULL,'chunk');
/*!40000 ALTER TABLE `file_informations` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `person`
--

DROP TABLE IF EXISTS `person`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `person` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `face_hash` varchar(1024) COLLATE utf8_unicode_ci NOT NULL,
  `voice_hash` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `avatar` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `name` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `person`
--

LOCK TABLES `person` WRITE;
/*!40000 ALTER TABLE `person` DISABLE KEYS */;
INSERT INTO `person` VALUES (1,'','','','user59ce15ceb862a','2017-09-29 09:43:42','2017-09-29 09:43:42'),(2,'','','','user59ce15cec0529','2017-09-29 09:43:42','2017-09-29 09:43:42'),(3,'','','','user59ce15cec0d02','2017-09-29 09:43:42','2017-09-29 09:43:42'),(4,'','','','user59ce15cec1443','2017-09-29 09:43:42','2017-09-29 09:43:42'),(5,'','','','user59ce15cec1bff','2017-09-29 09:43:42','2017-09-29 09:43:42'),(6,'','','','user59ce15cec23a9','2017-09-29 09:43:42','2017-09-29 09:43:42'),(7,'','','','user59ce15cec2b94','2017-09-29 09:43:42','2017-09-29 09:43:42'),(8,'','','','user59ce15cec3370','2017-09-29 09:43:42','2017-09-29 09:43:42'),(9,'','','','user59ce15cec3aee','2017-09-29 09:43:42','2017-09-29 09:43:42'),(10,'','','','user59ce15cec42bb','2017-09-29 09:43:42','2017-09-29 09:43:42');
/*!40000 ALTER TABLE `person` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `project_counter`
--

DROP TABLE IF EXISTS `project_counter`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `project_counter` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `project_id` int(11) NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `project_counter`
--

LOCK TABLES `project_counter` WRITE;
/*!40000 ALTER TABLE `project_counter` DISABLE KEYS */;
/*!40000 ALTER TABLE `project_counter` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `scene`
--

DROP TABLE IF EXISTS `scene`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `scene` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `person_id` int(11) NOT NULL,
  `type` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `broadcast_id` int(11) NOT NULL,
  `started_at` timestamp NULL DEFAULT NULL,
  `finished_at` timestamp NULL DEFAULT NULL,
  `text` longtext COLLATE utf8_unicode_ci,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `scene`
--

LOCK TABLES `scene` WRITE;
/*!40000 ALTER TABLE `scene` DISABLE KEYS */;
/*!40000 ALTER TABLE `scene` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `video_stream`
--

DROP TABLE IF EXISTS `video_stream`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `video_stream` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `stream_id` varchar(100) COLLATE utf8_unicode_ci NOT NULL,
  `stream_key` varchar(100) COLLATE utf8_unicode_ci NOT NULL,
  `stream_access_key` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `url` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `file_path` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `provider` varchar(255) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'facecast',
  `initialization_status` varchar(100) COLLATE utf8_unicode_ci NOT NULL,
  `preview_generation_status` varchar(100) COLLATE utf8_unicode_ci NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `video_stream`
--

LOCK TABLES `video_stream` WRITE;
/*!40000 ALTER TABLE `video_stream` DISABLE KEYS */;
/*!40000 ALTER TABLE `video_stream` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;