from flask import Flask, jsonify
from flask_cors import CORS
import subprocess

import mysql.connector

import os
import sys

currentPath = os.path.dirname(os.path.abspath(__file__))

rootDir = os.path.dirname(currentPath)
credentialsPath = os.path.join(rootDir, 'credentials')

sys.path.append(credentialsPath)
import mysqlcredentials as mc

app = Flask(__name__)
CORS(app)

@app.route("/api/data")
def getData():    
    connection = mysql.connector.connect(
        user = mc.user,
        password = mc.password,
        host = mc.host,
        database = mc.database
    )

    cursor = connection.cursor(dictionary=True)
    cursor.execute("SELECT * FROM initial")
    output = cursor.fetchall()

    cursor.close()
    connection.close()

    return jsonify(output)

@app.route('/api/refresh')
def refreshData():
    subprocess.run(["python", "createSQLdb.py"])

@app.route('/scrambler')
def scrambler():
    

if __name__ == '__main__':
    app.run(port=5000)