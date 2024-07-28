DELETE FROM `acore_string` WHERE `entry` IN (40000, 40001, 40002, 40003);
INSERT INTO `acore_string` (entry, content_default) VALUES (40000, 'Sold {}x {} for {}.');
INSERT INTO `acore_string` (entry, content_default) VALUES (40001, 'Sold a total of {} item(s) for {}.');
INSERT INTO `acore_string` (entry, content_default) VALUES (40002, 'Nothing to sell.');
INSERT INTO `acore_string` (entry, content_default) VALUES (40003, 'Invalid item quality received.');