# UDP сервер на Python
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', 1234))

print("UDP server started")
while True:
    data, addr = sock.recvfrom(1024)
    print(f"Received: {data.decode()} from {addr}")
    sock.sendto(b"UDP response from PC!", addr)