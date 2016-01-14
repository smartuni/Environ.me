#-----------------------------------------------------------
# Project: Environ.me
# Filename : views.py in Folder Environ.me\app
# Author: Thomas Fischer / Nicolas Albers
# Date: 2016_01
#
# Get sensor-values from mysql-DB and uses FLASK to send
# the values to html-templates 
#-----------------------------------------------------------
from flask import Flask
from flask import render_template
from flask import request
from app import app
import datetime
import time
import mysql.connector

# Get values for details.html and open it
@app.route('/details')
def details():
    now = datetime.datetime.now()
    timeString = now.strftime("%Y-%m-%d %H:%M")
    db = mysql.connector.connect(user='monitor',password='Raspberry',host='localhost',database='Environme')
    curs = db.cursor()
	#Get temperatures sorted by DateTime Descending
    curs.execute ("SELECT sensor_id, dt, temperature FROM temperatures WHERE sensor_id=%s ORDER BY dt DESC", (str(1),))
    values1 = curs.fetchall()
    l = 10
    labels=[]
    temperatures=[]
    for i in range(10):
        labels.append(1)
        labels[i] = ''
        temperatures.append(1)
        temperatures[i] = -30000;
        print(i)
	#Get the last 10 temperatures and labels
    for value1 in values1:
        if (i>=0):
            labels[i] = value1[1]
            temperatures[i] = value1[2]
            print(i)
            i = i - 1
    curs.execute ("SELECT sensor_id, dt, temperature FROM temperatures WHERE sensor_id=%s ORDER BY dt DESC", (str(2),))
    values2 = curs.fetchall()
    labels2=[]
    temperatures2=[]
    for i in range(10):
        labels2.append(1)
        labels2[i] = ''
        temperatures2.append(1)
        temperatures2[i] = -30000;
        print(i)
    for value2 in values2:
        if (i>=0):
            labels2[i] = datetime.datetime.strptime(value2[1], "%Y-%m-%d %H:%M")
            #labels2[i] = (((((labels2[i].year-1970) * 365 + labels2[i].day) * 24 + labels2[i].hour)* 60 +labels2[i].minute)* 60 + labels2[i].second)*10
            temperatures2[i] = value2[2]
            print(i)
            print(labels2[i])
            i = i - 1
    curs.close()
    db.close()
	#Problems with FLASK and arrays -> only 10 values for each sensor 
    return render_template('details.html',
                            title ='Environ.me!Details',
                            time = timeString,
							label10 = labels2[0],
							label11 = labels2[1],
							label12 = labels2[2],
							label13 = labels2[3],
							label14 = labels2[4],
							label15 = labels2[5],
							label16 = labels2[6],
							label17 = labels2[7],
							label18 = labels2[8],
							label19 = labels2[9],
                            temp10 = temperatures[0],
                            temp11 = temperatures[1],
                            temp12 = temperatures[2],
							temp13 = temperatures[3],
							temp14 = temperatures[4],
							temp15 = temperatures[5],
							temp16 = temperatures[6],
							temp17 = temperatures[7],
							temp18 = temperatures[8],
							temp19 = temperatures[9],
							temp20 = temperatures2[0],
                            temp21 = temperatures2[1],
                            temp22 = temperatures2[2],
							temp23 = temperatures2[3],
							temp24 = temperatures2[4],
							temp25 = temperatures2[5],
							temp26 = temperatures2[6],
							temp27 = temperatures2[7],
							temp28 = temperatures2[8],
							temp29 = temperatures2[9],
							l = l
                           )

# Button 'updateLED_off1': Update sensor1-led-status to 0 and call index()
@app.route('/updateLED_off1')
def updateLED_off1():
    db = mysql.connector.connect(user='monitor',password='Raspberry',host='localhost',database='Environme')
    curs = db.cursor()
    curs.execute("UPDATE sensors SET led = 0 WHERE id=1;")
    db.commit()
    curs.close()
    db.close()
    return index()

# Button 'updateLED_off2: Update sensor2-led-status to 0 and call index()
@app.route('/updateLED_off2')
def updateLED_off2():
    db = mysql.connector.connect(user='monitor',password='Raspberry',host='localhost',database='Environme')
    curs = db.cursor()
    curs.execute("UPDATE sensors SET led=0 WHERE id=2")
    db.commit()
    curs.close()
    db.close()
    return index()

# Button 'updateLED_left1: Update sensor1-led-status to 1 and call index()
@app.route('/writeLED_left1')
def updateLED_left1():
    db = mysql.connector.connect(user='monitor',password='Raspberry',host='localhost',database='Environme')
    curs = db.cursor()
    curs.execute("UPDATE sensors SET led=1 WHERE id=1")
    db.commit()
    curs.close()
    db.close()
    return index()

# Button 'updateLED_left2: Update sensor2-led-status to 1 and call index()
@app.route('/writeLED_left2')
def updateLED_left2():
    db = mysql.connector.connect(user='monitor',password='Raspberry',host='localhost',database='Environme')
    curs = db.cursor()
    curs.execute("UPDATE sensors SET led=1 WHERE id=2")
    db.commit()
    curs.close()
    db.close()
    return index()

# Button 'updateLED_right1': Update sensor1-led-status to 2 and call index()
@app.route('/writeLED_right1')
def updateLED_right1():
    db = mysql.connector.connect(user='monitor',password='Raspberry',host='localhost',database='Environme')
    curs = db.cursor()
    curs.execute("UPDATE sensors SET led=2 WHERE id=1")
    db.commit()
    curs.close()
    db.close()
    return index()

# Button 'updateLED_right2': Update sensor2-led-status to 2 and call index()
@app.route('/writeLED_right2')
def updateLED_right2():
    db = mysql.connector.connect(user='monitor',password='Raspberry',host='localhost',database='Environme')
    curs = db.cursor()
    curs.execute("UPDATE sensors SET led=2 WHERE id=2")
    db.commit()
    curs.close()
    db.close()
    return index()

# Get values for index.html and open it
@app.route('/')
@app.route('/index')
def index():
    now = datetime.datetime.now()
    timeString = now.strftime("%Y-%m-%d %H:%M")
    db = mysql.connector.connect(user='monitor',password='Raspberry',host='localhost',database='Environme')
    curs = db.cursor()

    curs.execute("SELECT id, ipv6, Position, led FROM sensors")
    rows = curs.fetchall()
    
    curs.execute ("SELECT sensor_id, dt, temperature FROM temperatures WHERE sensor_id=%s AND dt = (SELECT MAX(dt) FROM temperatures WHERE sensor_id = %s) ", (str(1),str(1)))
    values1 = curs.fetchall()
    
    curs.execute ("SELECT sensor_id, dt, temperature FROM temperatures WHERE sensor_id=%s AND dt = (SELECT MAX(dt) FROM temperatures WHERE sensor_id = %s) ", (str(2),str(2)))
    values2 = curs.fetchall()

    curs.execute ("SELECT sensor_id, dt, temperature FROM temperatures WHERE sensor_id=%s AND temperature = (SELECT MAX(temperature) FROM temperatures WHERE sensor_id = %s)", (str(1),str(1)))
    maxt1 = curs.fetchall()

    curs.execute ("SELECT sensor_id, dt, temperature FROM temperatures WHERE sensor_id=%s AND temperature = (SELECT MAX(temperature) FROM temperatures WHERE sensor_id = %s)", (str(2),str(2)))
    maxt2 = curs.fetchall()

    curs.close()
    db.close()

    return render_template('index.html',
                           title ='Environ.me!',
                           time = timeString,
                           rows = rows,
                           values1 = values1,
                           values2 = values2,
                           maxt1 = maxt1,
                           maxt2 = maxt2)