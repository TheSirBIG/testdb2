@ECHO OFF

IF NOT DEFINED MYSQL_USER (
  mysql -e "\. create_testdb2.sql"
  GOTO SCRIPT_END
)

mysql -u%MYSQL_USER% -p%MYSQL_PASSWORD% -e "\. create_testdb.sql"

:SCRIPT_END
