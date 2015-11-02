from http.server import BaseHTTPRequestHandler, HTTPServer
import time

hostName = "localhost"
hostPort = 9000

class MyServer(BaseHTTPRequestHandler):
	#while 1:
		def do_GET(self):
			self.send_response(200)
			self.send_header("Content-type", "text/html")
			self.end_headers()
			self.wfile.write(bytes("<html><head><title>Title goes here.</title>   <META HTTP-EQUIV=\"refresh\" CONTENT=\"60\">    </head>     ", "utf-8"))
			self.wfile.write(bytes("<body><p>This is a test.</p>", "utf-8"))
			self.wfile.write(bytes('<button onclick>Click Me!</button> <!-- Example 2 --> <button onclick="JavaScript:alert(\'You will love this book!\')">	<img src="/pix/web_graphics/free_website_graphics/icons/books/book13.gif" alt="Read book"><br>Read Book! </button>',"utf-8"))
			self.wfile.write(bytes("<p>You accessed path: %s</p>" % self.path, "utf-8"))
			self.wfile.write(bytes("<p>Uhrzeit %s</p>" % time.asctime(), "utf-8"))	


			self.wfile.write(bytes("</body></html>", "utf-8"))
			#time.sleep( 5 )
while 1:
	myServer = HTTPServer((hostName, hostPort), MyServer)
	print(time.asctime(), "Server Starts - %s:%s" % (hostName, hostPort))

	try:
	    myServer.serve_forever()
	except KeyboardInterrupt:
	    pass
	time.sleep(5)

myServer.server_close()
print(time.asctime(), "Server Stops - %s:%s" % (hostName, hostPort))
