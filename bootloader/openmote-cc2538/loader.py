#!/usr/bin/env python3
import glob
import os
import subprocess
import sys
from threading import Thread, Semaphore

if os.name == 'nt':  # Windows
    PYTHON_PY = 'python '
elif os.name == 'posix':  # Linux
    PYTHON_PY = 'python3 '


class BootloadThread(Thread):
    def __init__(self, port, hex_file, sem, script_path):
        # store params
        super().__init__()

        self.com_port = port
        self.hex_file = hex_file
        self.counting_sem = sem
        self.script_path = script_path

        # initialize parent class
        self.name = 'BootloadThread{0}'.format(self.com_port)

    def run(self):
        print('Start bootloading on {0}'.format(self.com_port))
        subprocess.call(PYTHON_PY + os.path.join(script_path, 'cc2538-bsl.py') +
                        ' -e --bootloader-invert-lines -w -b 400000 -p {0} {1}'.format(self.com_port, self.hex_file),
                        shell=True)

        print('Done bootloading on {0}'.format(self.com_port))

        # indicate done
        self.counting_sem.release()


def expand_port_list(ports):
    # Process only when there is a single port
    if len(ports) == 1:
        port = ports[0]
        ports = []
        last_char = port[-1:]
        base_dir = os.path.dirname(port)

        # /dev/ttyUSBX means bootload all ttyUSB ports
        if last_char == "X":
            base_file = os.path.basename(port[:-1])
            ports = sorted(glob.glob(os.path.join(base_dir, base_file) + "*"))

        # /dev/ttyUSB[1-2] means bootload a range of ttyUSB ports
        elif last_char == "]":
            base_file = os.path.basename(port.split('[')[0])
            first, last = sorted(map(int, ((port.split('['))[1].split(']')[0]).split('-')))

            # For all elements in range
            for i in range(first, last + 1):
                p = os.path.join(base_dir, base_file + str(i))
                ports.append(p)
        else:
            ports = [port]

    # Check if new list is empty
    if not ports:
        print("Bootload port expansion is empty or erroneous!")
        sys.exit(-1)

    return ports


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Incorrect number of arguments")
        sys.exit(-1)

    bootload_threads = []
    counting_sem = Semaphore(0)

    # Enumerate ports
    script_path = os.path.split(sys.argv[0])[0]
    com_ports = [sys.argv[1]]
    bin_file = sys.argv[2]

    # Check com_ports to bootload
    com_ports = expand_port_list(com_ports)

    # create threads
    for com_port in com_ports:
        bootload_threads += [
            BootloadThread(
                port=com_port,
                hex_file=bin_file,
                sem=counting_sem,
                script_path=script_path
            )
        ]

    # start threads
    for t in bootload_threads:
        t.start()
    # wait for threads to finish
    for t in bootload_threads:
        counting_sem.acquire()
