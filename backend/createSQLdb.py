import gspread
import mysql.connector

import os
import sys

currentPath = os.path.dirname(os.path.abspath(__file__))

rootDir = os.path.dirname(currentPath)
credentialsPath = os.path.join(rootDir, 'credentials')

sys.path.append(credentialsPath)
import mysqlcredentials as mc

serviceAccountPath = os.path.join(credentialsPath, 'hics-466603-b3bc066db823.json')
client = gspread.service_account(filename=serviceAccountPath)

def fetchData(sheetName, worksheetIndex):
    sheet = client.open_by_url(sheetName).get_worksheet(worksheetIndex)
    return sheet.get_all_values()

def createSQLdb():
    connection = mysql.connector.connect(
        user = mc.user,
        password = mc.password,
        host = mc.host
    )

    cursor = connection.cursor()
    cursor.execute("CREATE DATABASE IF NOT EXISTS hics")
    
    cursor.close()
    connection.close()

def mySQLWrite(sqlData, table):
    try:
        connection = mysql.connector.connect(
            user = mc.user,
            password = mc.password,
            host = mc.host,
            database = mc.database
        )

        sqlDropTable = f" DROP TABLE IF EXISTS {table} "

        sqlCreateTable = f"""CREATE TABLE {table}(
            id INT AUTO_INCREMENT PRIMARY KEY,
            Members VARCHAR(255),
            Situation_Analyst INT,
            Solutions INT,
            Kabyas_Lapdog INT,
            Financials INT
        )"""

        sqlInsert = f"""INSERT INTO {table}(
            Members,
            Situation_Analyst,
            Solutions,
            Kabyas_Lapdog,
            Financials ) 
            VALUES (%s,%s,%s,%s,%s)"""
        
        cursor = connection.cursor()
        cursor.execute(sqlDropTable)
        cursor.execute(sqlCreateTable)

        for i in sqlData:
            trimmed = [i[0].strip(),] + list(i[1:])
            cursor.execute(sqlInsert, trimmed)

        cursor.execute("""SELECT COUNT(*) 
                       FROM information_schema.tables 
                       WHERE table_schema = 'hics' 
                       AND table_name = 'past_history'""")
        
        if cursor.fetchone()[0] == 0:
            cursor.execute("CREATE TABLE IF NOT EXISTS past_history (id INT NOT NULL, past_match VARCHAR(255))")
            
            insertTable = "INSERT INTO past_history (id, past_match) VALUES "
            insertTable += ", ".join(f"({i + 1}, '')" for i in range(48))

            cursor.execute(insertTable)

        connection.commit()

    except mysql.connector.Error as error:
        print("Error. Table not updated:", error)
        connection.rollback()

    finally:
        cursor.close()
        connection.close()

def NULLValues(ls):
    for x in range(len(ls)):
        for y in range(len(ls[x])):
            if ls[x][y] == '':
                ls[x][y] = None

data = fetchData(mc.url, 0)

createSQLdb()
NULLValues(data)
mySQLWrite(data, 'initial')


