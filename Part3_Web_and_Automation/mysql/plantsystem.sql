-- phpMyAdmin SQL Dump
-- version 5.1.0
-- https://www.phpmyadmin.net/
--
-- Host: 192.168.0.25:3306
-- Generation Time: Oct 22, 2021 at 02:07 AM
-- Server version: 8.0.24
-- PHP Version: 7.4.16

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `plantsystem`
--

-- --------------------------------------------------------

--
-- Table structure for table `sensor_data`
--

CREATE TABLE `sensor_data` (
  `id` int NOT NULL,
  `insertTime` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `address` tinyint UNSIGNED NOT NULL,
  `T` float UNSIGNED NOT NULL,
  `M` smallint UNSIGNED NOT NULL,
  `A` smallint UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Sensor data';

-- --------------------------------------------------------

--
-- Table structure for table `sensor_links`
--

CREATE TABLE `sensor_links` (
  `id` int UNSIGNED NOT NULL,
  `plantName` varchar(25) NOT NULL COMMENT 'Plant name',
  `sensorAddress` tinyint UNSIGNED NOT NULL COMMENT 'I2C address of sensor attached to this plant',
  `solenoidAddress` tinyint UNSIGNED NOT NULL COMMENT 'I2C address of solenoid attached to this plant'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- --------------------------------------------------------

--
-- Table structure for table `watering_data`
--

CREATE TABLE `watering_data` (
  `id` int UNSIGNED NOT NULL,
  `solenoidAddress` tinyint UNSIGNED NOT NULL,
  `waterVolume` smallint UNSIGNED NOT NULL,
  `timestamp` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `sensor_data`
--
ALTER TABLE `sensor_data`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `sensor_links`
--
ALTER TABLE `sensor_links`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `watering_data`
--
ALTER TABLE `watering_data`
  ADD PRIMARY KEY (`id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `sensor_data`
--
ALTER TABLE `sensor_data`
  MODIFY `id` int NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `sensor_links`
--
ALTER TABLE `sensor_links`
  MODIFY `id` int UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `watering_data`
--
ALTER TABLE `watering_data`
  MODIFY `id` int UNSIGNED NOT NULL AUTO_INCREMENT;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
