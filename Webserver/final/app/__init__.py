from flask import Flask
import mysql.connector

app = Flask(__name__)
db = mysql.connector.connect(user='monitor',password='Raspberry',host='localhost',database='Environme')

from app import views