#!/usr/bin/python2.7 -tt
import socket

HOST = '127.0.0.1'

def listen(s):
    data = s.recv()
    while data != 0:
        print data
        data.send("GOT IT!")
        data = s.recv()
        
def connect_to_lb(remote_port):
    assert remote_port.isdigit
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind(())
        s.connect(HOST, remote_port))
        listen(s)
    except Exception as e:
        print "Error, couldn't open socket", e
        raise e
    finally:
        s.close()
        
def main():
    import sys
    if len(sys.argv) != 2:
        print('Usage: %s <Remote load balancer port>' % sys.argv[0])
    connect_to_lb(sys.argv[1])
if __name__ == "__main__":
    main()
