#!/usr/bin/env python3

#import MySQLdb

import mysql.connector
#from mysql.connector import errorcode

#try:
db = mysql.connector.connect(user='monitor',password='Raspberry',host='localhost',database='Environme')
#except mysql.connector.Error as err:
#   if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
#      print("Something is wrong with your user name or password")
#   elif err.errno == errorcode.ER_BAD_DB_ERROR:
#      print("Database does not exist")
#   else:
#      print(err)
#else:
#   db.close()


curs=db.cursor()

curs.execute("SELECT id, ipv6, Position, led FROM sensors")

print("\nid, ipv6,				Position, led")
print("===========================================================")


#for reading in curs.fetchall():
for (id, ipv6, Position, led) in curs:
   print("{}, {}, {}, {}".format(id, ipv6, Position, led))
   #print(str(reading[0])+"	"+str(reading[1])+" 	"+reading[2]+"  	"+str(reading[3])

curs.execute ("SELECT id, sensor_id, dt, temperature FROM temperatures WHERE sensor_id>%s", (str(1),))
print("\nid, sensor_id, Date Time,  temperature")
print("========================================")

for (id, sensor_id, dt, temperature) in curs:
   print("{}, {}, {}, {}".format(id, sensor_id, dt, temperature))

curs.close()
db.close()
