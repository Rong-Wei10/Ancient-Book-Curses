import sys
import socket
import struct

PING = 0
SINGLE = 1
MULTI = 2
PROTOCOL = 13
HEADERSIZE = 12

def make_req(op, numargs, arg):
    net_id_bytes = int(PROTOCOL).to_bytes(4, 'big')
    net_op_bytes = op.to_bytes(4, 'big')
    net_args_bytes = numargs.to_bytes(2, 'big')
    req = net_id_bytes + net_op_bytes + net_args_bytes
    if (numargs > 0):
        req += arg.to_bytes(4, 'big')
    return req


def main():
    host = sys.argv[1]
    port = int(sys.argv[2])
    num = int(sys.argv[3])

    sock = socket.socket(family=socket.AF_INET)
    sock.settimeout(1)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER,
                             struct.pack('ii', 1, 0))

    info = socket.getaddrinfo(host, port, family=socket.AF_INET)
    addr = info[0][4]

    sock.connect(addr)
    print("CONNECTION SUCCESSFUL " + host + " " + str(port))

    if num == 0:
        op = PING
        arg = 0
    elif num == 1:
        op = SINGLE
        arg = 0
    elif num == 13:   # 13 is a special error code with bad op
        op = 66
        num = 0
        arg = 5
    else:
        op = MULTI
        arg = num

    req = make_req(op, num, arg)
    sock.send(req)

    data = sock.recv(HEADERSIZE)
    print("RECEIVED HEADER")

    # Decrypt header -- don't' do this in real life with hard coded constants
    id = data[3]
    status = data[7]
    len = data[11] + (data[10] << 8) + (data[9] << 16) + (data[8] << 24)

    print("PROTOCOL: " + str(id))
    print("STATUS: " + str(status))
    print("PAYLOAD: ")

    data = sock.recv(len)
    print(data.decode('utf-8'))

# if __name__ == "__main__":
#     main()
