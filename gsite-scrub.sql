--
-- This is the SQL used to scrub a live Gardens database for use on the
-- Gardens test site (gsteamer). As a developer, you may need to edit this
-- file to guard against test systems exposing private data or performing
-- something they shouldn't.
--
-- Scrubbing that only needs to run for local developer downloads of a Gardens
-- database (not on gsteamer) should be in gsite-scrub-local-dev.sql, not here.
--

-- -----------------------------------------------------------------------------
-- This initial block of code should only contain database tables that we
-- expect to be required on Gardens sites. The scrub script will fail if any of
-- these tables do not exist.
-- -----------------------------------------------------------------------------

-- Scrub user mails -- set a wacky subdomain to avoid gardener-gardens collisions of munged addresses.
UPDATE `users` SET mail = CONCAT('user', uid, '@', MD5(mail), '.example.com'), init = CONCAT('user', uid, '@', MD5(init), '.example.com') WHERE uid > 0;

-- Remove any existing OpenID associations. --
DELETE FROM openid_association;

-- Scrub variables --
DELETE FROM `variable` WHERE `name` = 'site_mail';
INSERT INTO `variable` (name, value) VALUES ('site_mail', 's:26:\"steamer-gardens@acquia.com\";');
DELETE FROM `variable` WHERE `name` = 'gardens_misc_ga_tracking_code';
INSERT INTO `variable` (name, value) VALUES ('gardens_misc_ga_tracking_code', 's:0:\"\";');
DELETE FROM `variable` WHERE `name` = 'cron_key';
INSERT INTO `variable` (name, value) VALUES ('cron_key', CONCAT('s:32:\"', MD5(RAND()), '\";'));
DELETE FROM `variable` WHERE `name` = 'drupal_private_key';
DELETE FROM `variable` WHERE `name` = 'gardens_client_gardener_data';
DELETE FROM `variable` WHERE `name` = 'gardens_misc_standard_domain';
DELETE FROM `variable` WHERE `name` = 'antivirus_settings_clamavdaemon';
-- We still want to send data to statsd, but marked as 'dev'
UPDATE `variable` SET `value` = 's:11:"gardens.dev";' WHERE `name` = 'gardens_statsd_prefix';
UPDATE `variable` SET `value` = 'i:0;' WHERE `name` = 'gardens_statsd_env_checked';
-- For image generation to work we need to both remove the set file path
-- and also force a menu rebuild.
DELETE FROM `variable` WHERE `name` = 'file_public_path';
DELETE FROM `variable` WHERE `name` = 'menu_rebuild_needed';
INSERT INTO `variable` (name, value) VALUES ('menu_rebuild_needed', 'b:1;');
-- We don't have to regenerate this if we don't want since Drupal will
-- auto-regenerate it if it's empty, but it doesn't hurt to either.
INSERT INTO `variable` (name, value) VALUES ('drupal_private_key', CONCAT('s:32:\"', MD5(RAND()), '\";'));

-- Force everything to refresh
DELETE FROM `variable` WHERE `name` = 'sqbs_flush_all_caches';
INSERT INTO `variable` (name, value) VALUES ('sqbs_flush_all_caches', 'b:1;');

-- Set restored sites to have gardens_devel set so that js errors spit out alerts
DELETE FROM `variable` WHERE `name` = 'gardens_devel';
INSERT INTO `variable` (name, value) VALUES ('gardens_devel', 'b:1;');

-- Set error reporting to default - display to the screen.
DELETE FROM `variable` WHERE `name` = 'error_level';

-- Scrub Janrain Engage app data.
DELETE FROM variable WHERE `name` LIKE 'rpx_%';

-- Scrub Akamai settings --
DELETE FROM `variable` WHERE `name` LIKE 'akamai_%';

-- Scrub domain_301_redirect settings --
DELETE FROM `variable` WHERE `name` LIKE 'domain_301_redirect_%';

-- Clear sessions and required cache tables that might have stale data.
TRUNCATE `sessions`;
TRUNCATE `themebuilder_session`;
TRUNCATE `cache`;
TRUNCATE `cache_bootstrap`;
TRUNCATE `cache_field`;
TRUNCATE `cache_filter`;
TRUNCATE `cache_form`;
TRUNCATE `cache_menu`;
TRUNCATE `cache_page`;
TRUNCATE `cache_path`;

-- -----------------------------------------------------------------------------
-- Define a stored procedure that ignores any "table does not exist" errors
-- from MySQL, since these occur whenever we try to scrub a database table that
-- might not exist on this particular Gardens site (because the site does not
-- have the corresponding module installed).
--
-- This block of code should therefore only contain database tables that we
-- expect to be optional on Gardens sites.
-- -----------------------------------------------------------------------------
DROP PROCEDURE IF EXISTS `gardensscruboptionalmodules`;
DELIMITER //
CREATE PROCEDURE gardensscruboptionalmodules ()
BEGIN
DECLARE CONTINUE HANDLER FOR SQLSTATE '42S02' BEGIN END;

-- Scrub comment mails--
UPDATE comment SET mail = CONCAT('commenter-on-cid-', cid, '@example.com') WHERE mail IS NOT NULL;

-- Send contact submissions to a dedicated address. --
UPDATE contact SET recipients = 'steamer-gardens@acquia.com';

-- Scrub any mailing list emails. --
UPDATE mailing_list_emails SET mail = CONCAT('mailing-list-email-', eid, '@example.com');

-- Clear out identifiable info from the poll_vote table (note this is part of
-- the primary key so it must remain unique).
UPDATE poll_vote SET hostname = MD5(hostname);

-- Clear cache tables from optional modules.
TRUNCATE `cache_block`;
TRUNCATE `cache_file_styles`;
TRUNCATE `cache_image`;
TRUNCATE `cache_media_xml`;
TRUNCATE `cache_styles`;
TRUNCATE `watchdog`;
TRUNCATE `mailhandler_mailbox`;
TRUNCATE `mailhandler_singlemailbox_addresses`;

-- -----------------------------------------------------------------------------
-- Close and execute the stored procedure defined above.
-- -----------------------------------------------------------------------------
END;
//
DELIMITER ;
CALL gardensscruboptionalmodules();

-- Mark this database as having been scrubbed.
DELETE FROM `variable` WHERE `name` = 'gsite_database_scrubbed';
INSERT INTO `variable` (name, value) VALUES ('gsite_database_scrubbed', 's:8:\"scrubbed\";');
