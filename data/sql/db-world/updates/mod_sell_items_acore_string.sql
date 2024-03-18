DELETE FROM `acore_string` WHERE `entry` IN (40000, 40001, 40002, 40003);
INSERT INTO `acore_string` (entry, content_default) VALUES (40000, 'Sold %dx %s for %s.');
INSERT INTO `acore_string` (entry, content_default) VALUES (40001, 'Sold a total of %d item(s) for %s.');
INSERT INTO `acore_string` (entry, content_default) VALUES (40002, 'Nothing to sell.');
INSERT INTO `acore_string` (entry, content_default) VALUES (40003, 'Invalid item quality received.');