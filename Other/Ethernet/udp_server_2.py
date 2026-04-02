# UDP сервер на Python
import socket
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', 1234))
sock.settimeout(5.0)  # Таймаут 5 секунд

print("UDP server started on port 1234")
print("Waiting for W5200 packets...")

packet_count = 0
while True:
    try:
        data, addr = sock.recvfrom(1024)
        packet_count += 1
        print(f"\n--- Packet #{packet_count} ---")
        print(f"From: {addr}")
        print(f"Data: {data.decode()}")
        print(f"Length: {len(data)} bytes")
        
        # Отправляем ответ
        response = f"Response from PC to packet #{packet_count}"
        sock.sendto(response.encode(), addr)
        print(f"Sent response: {response}")
        
    except socket.timeout:
        print(".", end="", flush=True)
        time.sleep(1)
    except KeyboardInterrupt:
        print("\nServer stopped")
        break
    except Exception as e:
        print(f"\nError: {e}")

sock.close()