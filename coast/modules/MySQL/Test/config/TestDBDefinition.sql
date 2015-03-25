-- Definition for the test data base

DROP Database IF EXISTS ##database##;
Create Database ##database##;
use ##database##;

Create TABLE TestTable (
	Name VARCHAR(10) NOT NULL PRIMARY KEY,
	Number INT UNSIGNED NOT NULL
);

INSERT INTO TestTable VALUES ("Test",1);
INSERT INTO TestTable VALUES ("MySQL",2);
INSERT INTO TestTable VALUES ("Now",33);

