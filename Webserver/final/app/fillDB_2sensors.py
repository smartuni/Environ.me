#!/usr/bin/env python3

#import MySQLdb
import mysql.connector

#db = MySQLdb.connect("localhost", "monitor", "password", "Environme")
db = mysql.connector.connect(user='monitor',password='Raspberry',host='localhost',database='Environme')
curs = db.cursor()

# note that I'm using triplle quotes for formatting purposes
# you can use one set of double quotes if you put the whole string on one line
try:
    curs.execute ("""INSERT INTO sensors VALUES(NULL, 'fe80::5bb3:4e48:6fdc:6002', 'R07.61 BT7', 0);""")
    curs.execute ("""INSERT INTO sensors VALUES(NULL, 'fe80::f8e3:4e62:71ba:600a', 'Flur 7.Stock BT7', 0);""")

    #curs.execute ("""INSERT INTO sensorValues VALUES(NULL, 1, NOW(), 33.5, 40.1);""")
    #curs.execute ("""INSERT INTO sensorValues VALUES(NULL, 2, NOW(), 16.8, 44.1);""")

    #curs.execute ("""INSERT INTO temperatures VALUES(NULL, 1, NOW(), 2845);""")
    #curs.execute ("""INSERT INTO temperatures VALUES(NULL, 2, NOW(), 2517);""")

    #curs.execute ("""INSERT INTO humidity VALUES(NULL, 1, NOW(), 3969);""")
    #curs.execute ("""INSERT INTO humidity VALUES(NULL, 2, NOW(), 4210);""")

    db.commit()
    print("Data committed")

except:
    print("Error: the database is being rolled back")
    db.rollback()

