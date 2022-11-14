CREATE DATABASE testdb DEFAULT CHARACTER SET cp1251 COLLATE cp1251_general_ci;
USE testdb;

CREATE TABLE IF NOT EXISTS test_table1 (
  id bigint unsigned AUTO_INCREMENT PRIMARY KEY,
  date timestamp NOT NULL,
  value text
) ENGINE=MyISAM;
