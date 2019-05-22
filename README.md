#(EXTREMELY) BASIC (AND SENSITIVE) SMTP SERVER

**Setup**

First change the top macro definitions in server.c
```#define USERNAME "dummy"
#define PASSWORD "dummy"```

Create a Database name mails in your MySql server

And compile the files using this command(in the linux floder inside the repo)-
```gcc -I/usr/include/mysql -o server server.c `mysql_config --cflags --libs` && gcc -o client client.```

Run Server
```./server```

Run Client
```./client localhost```
OR
```./client <SERVER IP>```

You can even Telnet to the server though some commands may malfunction
```telnet localhost 42069```
OR
```telnet <SERVER IP> 42069```

Following commands have been implemented-
```HELO
MAIL FROM
RCPT TO
DATA
RSET
SEND
NOOP
QUIT
```

Frontend is currently in the making!

PS- you will see a LOT of debug messages on the server terminal so kindly ignore them for now