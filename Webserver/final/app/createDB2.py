#!/usr/bin/env python3

#import MySQLdb
import mysql.connector

#db = MySQLdb.connect("localhost", "monitor", "password", "Environme")
db = mysql.connector.connect(user='monitor',password='Raspberry',host='localhost',database='Environme')
curs = db.cursor()

# note that I'm using triplle quotes for formatting purposes
# you can use one set of double quotes if you put the whole string on one line
try:
    curs.execute ("""CREATE TABLE sensors (id INT AUTO_INCREMENT PRIMARY KEY, ipv6 VARCHAR(50), Position VARCHAR(50), led INT);""")
    curs.execute ("""CREATE TABLE temperatures(id INT AUTO_INCREMENT PRIMARY KEY, sensor_id INT, dt VARCHAR(16), temperature INT);""")
    curs.execute ("""CREATE TABLE humidity(id INT AUTO_INCREMENT PRIMARY KEY, sensor_id INT, dt VARCHAR(16), humidity INT);""")

    db.commit()
    print("Data committed")

except:
    print("Error: the database is being rolled back")
    db.rollback()

