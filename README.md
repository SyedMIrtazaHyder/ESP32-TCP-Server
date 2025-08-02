# ESP32-TCP-Server

A TCP server programmed on ESPIDF using the wifi and lwIP APIs. To test the server use netcat or run the `netcat.bash <esp32-ip>` script with the ESP32's ip address. For a client to exit just type `exit` on the client end, the server will perform all the necessary cleanup.

*NOTE:* Both the server and the netcat.bash script use the port 3000. To change the listening port kindly alter the port at ![server.h](main/server.h).

### TODO:
1. Make client connection exit after timer.
2. Update the script to make a HTTP server.
3. Setup a reverse proxy for the server.
