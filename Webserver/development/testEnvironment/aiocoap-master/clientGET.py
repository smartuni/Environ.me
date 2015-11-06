#!/usr/bin/env python3

# This file is part of the Python aiocoap library project.
#
# Copyright (c) 2012-2014 Maciej Wasilak <http://sixpinetrees.blogspot.com/>,
#               2013-2014 Christian Amsuess <c.amsuess@energyharvesting.at>
#
# aiocoap is free software, this file is published under the MIT license as
# described in the accompanying LICENSE file.

import logging
import asyncio

import sqlite3

import time


#conn = sqlite3.connect('example.db')
#connData = sqlite3.connect('data.db')
connTemperature = sqlite3.connect('temperature.db')

from aiocoap import *

logging.basicConfig(level=logging.INFO)

@asyncio.coroutine
def main():
    protocol = yield from Context.create_client_context()

    request = Message(code=GET)
   # request.set_request_uri('coap://localhost/time')


    request2 = Message(code=GET)
    #request2.set_request_uri('coap://localhost/other')
    request2.opt.uri_host = '127.0.0.1'
    request2.opt.uri_path=("other","block")

    request3 = Message(code=GET)
    request3.set_request_uri('coap://localhost/temperature')
#    request3.opt.uri_host = '127.0.0.1'
#    request3.opt.uri_host = 'fe80::f8e3:4e62:71ba:600a'
#    request3.opt.uri_path=("time")

	
    #try:
    #    response = yield from protocol.request(request).response
    #except Exception as e:
    #    print('Failed to fetch resource:')
    #    print(e)
    #else:
    #    print('Result: %s\n%r'%(response.code, response.payload))



    #try:
    #    response2 = yield from protocol.request(request2).response
    #except Exception as e:
    #    print('Failed to fetch resource2:')
    #    print(e)
    #else:
    #    print('Result: %s\n%r'%(response2.code, response2.payload))
    cTemperatureDB = connTemperature.cursor()
    try:
        response3 = yield from protocol.request(request3).response

    except Exception as e:
        print('Failed to fetch resource3:')
        print(e)
    else:
        print('Result: %s\n%r'%(response3.code, response3.payload))


        
		# Create table
        try:
            cTemperatureDB.execute('''CREATE TABLE temperature(sensorID text, time text, temperature text, humidity text)''')
        except Exception as e:
        	print('Failed to CREATE TABLE temperature ')
        	print(e)

        insert = 'INSERT INTO temperature VALUES (?,?,?,?)'

        test3 = response3.payload.decode("utf-8")
        sensorID = "1"
        ti = time.asctime() 
        humidity = "50"
        # Insert a row of data
        try:
        	cTemperatureDB.execute(insert, (sensorID,ti, test3, humidity))
        except Exception as e:
        	print('Failed to Insert:')
        	print(e)

	# Save (commit) the changes
        connTemperature.commit()

	# We can also close the connection if we are done with it.
	# Just be sure any changes have been committed or they will be lost.
        connTemperature.close()
		
		
		
		
		
#    c = conn.cursor()
#    for row in c.execute('SELECT * FROM stocks ORDER BY price'):
#    	print(row)

    #c = connData.cursor()
    #for row in c.execute('SELECT * FROM room'):
    #	print(row)
    	    	    	

if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(main())
