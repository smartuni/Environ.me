#!/usr/bin/env python3

#import MySQLdb
import mysql.connector

#db = MySQLdb.connect("localhost", "monitor", "password", "Environme")
db = mysql.connector.connect(user='monitor',password='Raspberry',host='localhost',database='Environme')
curs = db.cursor()

# note that I'm using triplle quotes for formatting purposes
# you can use one set of double quotes if you put the whole string on one line
try:
    #curs.execute ("""INSERT INTO sensors VALUES(NULL, '3001:0db8:85a3:08d3:1319:8a2e:0370:7344', 'Flur 7.Stock BT7', 0);""")

    #curs.execute ("""INSERT INTO sensorValues VALUES(NULL, 1, NOW(), 26.5, 40.7);""")
    #curs.execute ("""INSERT INTO sensorValues VALUES(NULL, 2, NOW(), 17.0, 44.0);""")

    curs.execute ("""INSERT INTO temperatures VALUES(NULL, 1, NOW(), 2360);""")
    curs.execute ("""INSERT INTO temperatures VALUES(NULL, 2, NOW(), 1747);""")

    curs.execute ("""INSERT INTO humidity VALUES(NULL, 1, NOW(), 4089);""")
    curs.execute ("""INSERT INTO humidity VALUES(NULL, 2, NOW(), 4328);""")

    db.commit()
    print("Data committed")

except:
    print("Error: the database is being rolled back")
    db.rollback()

