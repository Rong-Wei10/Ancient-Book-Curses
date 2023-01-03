import sys
import os
import time
import signal


# Spawn a server, wait 1 second, then spawn the client
def main():
    servercmd = sys.argv[1]
    clientcmd = sys.argv[2]

    servpid = os.fork()
    if servpid == 0:
        # child... run server
        servretval = os.system(servercmd)
        sys.exit(servretval)

    time.sleep(1)
    retval = os.system(clientcmd)

    time.sleep(1)
    servstatus = (os.wait()[1] >> 8)

    if retval != 0 or servstatus != 0:
        raise RuntimeError()

if __name__ == "__main__":
    main()
