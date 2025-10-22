## COS 460/540 - Computer Networks  
# Project 2: Simple HTTP Web Server  

## Silas Qualls  

This project is written in C on Linux.  

## How to Compile  

gcc -o httpserver project2.c -pthread

## How to Run

./httpserver (port) (document root)

(My example one)

./httpserver 1029 ./www

This will start the server on port 1029 and serve files from the www directory.

To test it, open a browser on the same network and visit:

http://localhost:(port number)/

(With my example port)

http://localhost:1029/

I was SSHing through the school wasp computer so I had to use port forwarding for this to work.
On VSCode I just use ctrl shift P to open the command pallete and pick port forwarding and just selected the port I wanted and it worked.

## My Experience with This Project

For this project, I implemented a basic multithreaded web server in C. The server listens on a configurable port, serves static files from a user-specified directory, and should handle multiple clients simultaneously. I used sockets for communication and created a separate thread for each client connection.

The most challenging part was correctly parsing HTTP requests and trying to make sure that the server could handle concurrent connections. Another annoying part I should've dealt with before starting was doing this on my own laptop instead of through SSH so I wouldn't have to go through port forwarding even though it wasn't particularly difficult to
do that part.

I liked this project because it helped me deepen my understanding a bit better how web servers operate at a lower level and it also gave me more experience with sockets and also port forwarding which I hadn't tried using before I don't think. Seeing the images through the browser for the first time from my own server was also pretty neat.
