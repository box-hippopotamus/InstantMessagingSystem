/* This file was generated by ODB, object-relational mapping (ORM)
 * compiler for C++.
 */
CREATE DATABASE IF NOT EXISTS `IM`;
USE `IM`;
DROP TABLE IF EXISTS `message`;

CREATE TABLE `message` (
  `id` INT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `message_id` varchar(64) NOT NULL,
  `user_id` varchar(64) NOT NULL,
  `session_id` varchar(64) NOT NULL,
  `message_type` TINYINT UNSIGNED NOT NULL,
  `create_time` TIMESTAMP NULL,
  `content` TEXT NULL,
  `file_id` varchar(64) NULL,
  `file_name` varchar(128) NULL,
  `file_size` INT UNSIGNED NULL)
 ENGINE=InnoDB;

CREATE UNIQUE INDEX `message_id_i`
  ON `message` (`message_id`);

CREATE INDEX `session_id_i`
  ON `message` (`session_id`);
