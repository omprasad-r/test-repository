--
-- This is the SQL used to scrub a "gsteamer-scrubbed" database for use on a
-- local machine. As a developer, you may need to edit this file to guard
-- against test systems exposing private data or performing something they
-- shouldn't.
--
-- Scrubbing that also needs to run when copying a live Gardens database to a
-- test Hosting environment (such as gsteamer) should be in gsite-scrub.sql,
-- not here. The scrubbing in this file is intended to be performed for local
-- dev copies, on a database where that first level of scrubbing has already
-- run.

-- Scrub themebuilder screenshot service keys --
DELETE FROM `variable` WHERE `name` = 'themebuilder_screenshot_access_key';
DELETE FROM `variable` WHERE `name` = 'themebuilder_screenshot_private_key';

-- Make sure that the variable cache is cleared (just in case this script ever
-- needs to be run independently).
TRUNCATE `cache_bootstrap`;

-- Add on some extra SQL required to make the site usable.
-- This is a hash of the word 'admin'.
UPDATE users SET pass='$S$CX5niOnlVy6ctHZUXsk5z21Xj.B7.Clr7QhgU9cZvZEI15ie2Wsi';
UPDATE users SET name='admin2' WHERE name = 'admin';
UPDATE users SET name='admin' WHERE name LIKE 'Gardens admin';
