DELETE FROM `command` WHERE `name`='sell';
INSERT INTO `command`(`name`, `security`, `help`) VALUES ('sell', 0, 'Syntax: .sell #quality
 Automatically sells all items of a specified quality (for example .sell gray will sell all gray items).
 Check the mod config file for the required strings and more.');
 