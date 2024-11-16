# Readme

Nothing special. A very basic API. Easy to use, easy to understand, easy to play.

## Undestanding socket states

The program example can be a valid help for undestanding what is happening under the hood. Let's now investigate about sockets state on a linux machine!

I will use the [ss](https://man7.org/linux/man-pages/man8/ss.8.html) program in these two manners

```bash
ss -l | grep 'Netid\|1234'
```

for investingating about the socket state of the listening server on port 1234. And

```bash
ss | grep 'Netid\|1234'
```

for a client / server connected each other and sending variable length strings.

## Step-by-step

### Connecting phase

First, launch the `./server` and start listening on port `1234`.

```
(ctrl+d to quit)
server > start 1234
```

```bash
ss -l | grep 'Netid\|1234'
Netid State  Recv-Q Send-Q    Local Address:Port    Peer Address:Port
tcp   LISTEN 0      5               0.0.0.0:1234         0.0.0.0:*
```

How curious, the Send-Queue is already filled with 5 bytes.

Second step, we proceed with `accept` command on the server. But I assure yout that nothing will change: without a timeout the call will just lock the server until a client decide to connect.

What is interesting, instead, is this scenario:

1. launch the server and open a socket with just only `start 1234`

2. launch the `./client` and `open` a connection to `localhost` on port `1234`

```bash
$ ss -l | grep 'Netid\|1234'
Netid State  Recv-Q Send-Q    Local Address:Port      Peer Address:Port
tcp   LISTEN 1      5               0.0.0.0:1234           0.0.0.0:*          
```

and 

```bash
$ ss | grep 'Netid\|1234'
Netid State  Recv-Q Send-Q    Local Address:Port    PeerAddress:Port
tcp   ESTAB  0      0             127.0.0.1:49514     127.0.0.1:1234          
tcp   ESTAB  0      0             127.0.0.1:1234      127.0.0.1:49514         
```

The client results connected even if the server didn't execute the `accept` call. And there is a byte pending on the Server Recv-Q.

Issuing `accept` on the server it will unlock immediatly and the byte will be gone

```bash
$ ss -l | grep 'Netid\|1234'
Netid State  Recv-Q Send-Q    Local Address:Port      Peer Address:Port
tcp   LISTEN 0      5               0.0.0.0:1234           0.0.0.0:*   
```

### Sending and receiving data

Client is connected. Let's send some data to the server

```
client > write Ciao
Sent 4 bytes
```

```bash
$ ss | grep 'Netid\|1234'
Netid State  Recv-Q Send-Q  Local Address:Port  Peer Address:Port
tcp   ESTAB  4      0           127.0.0.1:1234     127.0.0.1:57526        
tcp   ESTAB  0      0           127.0.0.1:57526    127.0.0.1:1234  
```

Nothing special. The server is ready to read the message pending on the Recv-Q. Issuing a `recv()` on the server

```
server > read
Read 4 bytes: Ciao
```

```bash
$ ss | grep 'Netid\|1234'
Netid State  Recv-Q Send-Q  Local Address:Port  Peer Address:Port
tcp   ESTAB  0      0           127.0.0.1:1234     127.0.0.1:57526        
tcp   ESTAB  0      0           127.0.0.1:57526    127.0.0.1:1234  
```

Still, nothing new. But the interesting thing when you watch the badly written `command_read()` function

```c++
void command_read(std::string const&)
{
  std::string buffer(256, 0);
  auto bytes = client.read(buffer);
  std::println("Read {} bytes: {}", bytes, buffer);
}
```

Even if you tried to read 256 bytes from the Recv-Q, the `recv()` function on the socket (called inside the `read()` method) will returns after cleared all 4 byte from the queue. The good thing is that the function will not get stuck until you read all 256 byte. Well, not a surprise. In fact, the man page about [recv(2)](https://man7.org/linux/man-pages/man2/recv.2.html) is clear

> The receive calls normally return any data available, up to the requested amount, rather than waiting for receipt of the full amount requested.

Now, what about when the buffer is smaller than the message sent by the other peer? Let's consider this edged scenario: a peer sent exactly 256 byte and from the other side we called `recv()` with the same amount; the number of byte read is the same of the number of byte we gave to the `recv()` function. Maybe there are some other bytes pending on the Recv-Q? We could try to invoke another `recv()` but if the Recv-Q is empty it will block, as `recv(2)` says

> If  no  messages are available at the socket, the receive calls wait for a message to arrive, unless the socket is nonblocking (see fcntl(2)), in which case the value -1 is re‐turned and errno is set to EAGAIN or EWOULDBLOCK.

We could attempt a call to `recv()` with the flag `MSG_DONTWAIT`: so the function will return `-1` with `errno` set to `EAGAIN` or `EWOULDBLOCK` if there are no other bytes to read. But this is not a robust choice. The peer could send another message but we could think that it is the continuation of the same message.

We need to know *a priori* the size of the message. This is not always possible. Most user cases include variabile size messages, like strings, different *fixed size* type of message, or even different *variable size* type of message. This is up to your application. The usual solution is, first, to send a fixed size *header* that include the number of bytes of the message and than the message itself. So the receiver can read safety a fixed number of byte, the header, and then use it to know how many bytes to read. I will discuss this solution later.

### Managing conncetion losses

Let's see what happen at the sockets when the client or the server close the connection prematurely.

#### Client is sending data peridiocally to a server

No bytes pending on the Recv-Q or Send-Q. If the client close the connection this is the result

```
client > close
Connection closed
```

```bash
$ ss | grep 'Netid\|1234'
Netid State      Recv-Q Send-Q  Local Address:Port  Peer Address:Port
tcp   CLOSE-WAIT 1      0           127.0.0.1:1234     127.0.0.1:57526        
tcp   FIN-WAIT-2 0      0           127.0.0.1:57526    127.0.0.1:1234
```

What's happen? According to [this](https://servicenow.iu.edu/kb?id=kb_article_view&sysparm_article=KB0023008)

1. the client sent a message with `FIN` bit set and put the socket state in `FIN-WAIT-1`

2. the server receive the message, put the socket into `CLOSE-WAIT` state, and send in response an `ACK`

3. the client receive the acknoledgment and put the socket state to `FIN-WAIT-2`

4. but, under TCP, the server still need to close the connection with a `close()` call, so these steps are needed to be excecuted by server side.

But what if the server try to read data from the client? Let's find out

```
server > read
Read 0 bytes:
```

```bash
$ ss | grep 'Netid\|1234'
Netid State      Recv-Q Send-Q  Local Address:Port  Peer Address:Port
tcp   CLOSE-WAIT 0      0           127.0.0.1:1234     127.0.0.1:57526        
tcp   FIN-WAIT-2 0      0           127.0.0.1:57526    127.0.0.1:1234
```

Reading 0 bytes usually means that the connection is closed. The pending byte on the Recv-Q disappear (?) and you can try to read how many times you want: the result will be the same before

```bash
server > read
Read 0 bytes:
server > read
Read 0 bytes:
server > read
Read 0 bytes:
```

Now close the connection by server side

```
server > close
Connection closed
```

```bash
$ ss | grep 'Netid\|1234'
Netid State      Recv-Q Send-Q  Local Address:Port  Peer Address:Port
```

Simply, an happy socket ending.

#### What if the server close the connection and the client try to send data?

Let's assume a client sending peridiocally data to a server. At some point the server close the connection.

Status:

```bash
Netid State      Recv-Q Send-Q  Local Address:Port  Peer Address:Port
tcp   CLOSE-WAIT 1      0           127.0.0.1:37748    127.0.0.1:1234
tcp   FIN-WAIT-2 0      0           127.0.0.1:1234     127.0.0.1:37748
```

the client try to send data:

```
client > write Ciao
Sent 4 bytes
```

```bash
$ ss | grep 'Netid\|1234'
Netid State      Recv-Q Send-Q  Local Address:Port  Peer Address:Port
```

another time

```
client > write Ciao
terminate called after throwing an instance of 'std::system_error'
  what():  write(): Broken pipe
Abort (core dumped)
```

that's because the socket is still open but the connection has ended. It might be a good idea to catch this exception and close the socket, then try to restabilish the connection.

Lesson: if the peer is in `CLOSE-WAIT`, because the other side calls `close()`, the first message sent will result almost like a `close()` call without closing the file descritor.