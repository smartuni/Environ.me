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

from aiocoap import *

logging.basicConfig(level=logging.INFO)

@asyncio.coroutine
def main():
    protocol = yield from Context.create_client_context()

    request = Message(code=GET)
    request.set_request_uri('coap://localhost/time')


    request2 = Message(code=GET)
    #request2.set_request_uri('coap://localhost/other')
    request2.opt.uri_host = '127.0.0.1'
    request2.opt.uri_path=("other","block")


    try:
        response = yield from protocol.request(request).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
    else:
        print('Result: %s\n%r'%(response.code, response.payload))



    try:
        response2 = yield from protocol.request(request2).response
    except Exception as e:
        print('Failed to fetch resource2:')
        print(e)
    else:
        print('Result: %s\n%r'%(response2.code, response2.payload))


if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(main())
