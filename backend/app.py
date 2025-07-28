from flask import Flask, jsonify
from flask_cors import CORS
import csv
import subprocess
import cpp

import mysql.connector

import os
import sys

currentPath = os.path.dirname(os.path.abspath(__file__))

rootDir = os.path.dirname(currentPath)
credentialsPath = os.path.join(rootDir, 'credentials')
cacheDir = os.path.join(rootDir, 'cache')

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
    cursor.execute("SELECT a.*, b.past_match " \
                   "FROM initial a, past_history b " \
                   "WHERE a.id = b.id")
        
    output = cursor.fetchall()

    cursor.close()
    connection.close()

    cache_path = os.path.join(cacheDir, 'cached_data.csv')
    with open(cache_path, mode='w', newline='', encoding='utf-8') as cache:
        writer = csv.DictWriter(cache, fieldnames=output[0].keys())
        writer.writerows(output)

    return jsonify(output)

@app.route('/api/refresh')
def refreshData():
    subprocess.run(["python", "createSQLdb.py"])
    return jsonify({"status": "Database refresh initiated"}), 200

@app.route('/api/scramble')
def scrambleData():
    cpp.scramble(mc.host, mc.user, mc.password)

    connection = mysql.connector.connect(
        user = mc.user,
        password = mc.password,
        host = mc.host,
        database = mc.database
    )

    cursor = connection.cursor(dictionary=True)
    cursor.execute("SELECT * FROM result")
    output = cursor.fetchall()

    cursor.close()
    connection.close()

    return jsonify(output)

@app.route("/api/update_history")
def updateDB():
    cpp.updateDB(mc.host, mc.user, mc.password)       
    return jsonify({"status": "Past match database updated"}), 200

if __name__ == '__main__':
    app.run(port=5000)