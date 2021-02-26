import os, sys, signal, argparse
import re
import subprocess

# size
def calc_size(fname):
    output = subprocess.check_output(['arm-none-eabi-size',fname]).decode('utf-8')
    for line in output.splitlines():
        # print(line)
        m = re.search('([0-9]+)\s+([0-9]+)\s+([0-9]+)',line)
        if m:
            text = int(m.group(1))
            data = int(m.group(2))
            bss  = int(m.group(3))
            # print("bss size according to size: \t%i" % int(m.group(3)))
            return [True, text, data, bss]

    return [False, -1, -1, -1]

def main(args):
    fsize = calc_size(args.file)
    if fsize[0]:
        text = fsize[1]
        data = fsize[2]
        bss = fsize[3]


        # print("Text: {} bytes".format(text))
        # print("Data: {} bytes".format(data))
        # print("BSS: {} bytes".format(bss))

        usedFlash = text + bss
        
        usedRAM = data + bss

        print("RAM used:  {:6} / {:6} ({:.2f}%)".format(usedRAM, args.ram, (usedRAM*100/args.ram) ) )
        print("FLASH used:{:6} / {:6} ({:.2f}%)".format(usedFlash, args.flash, (usedFlash*100/args.flash) ) )

# gracefully exit without a big exception message if possible
def ctrl_c_handler(signal, frame):
    print('Goodbye, cruel world!')
    exit(0)




def parse_size(size):
    # units = {"B": 1, "KB": 2**10, "MB": 2**20, "GB": 2**30, "TB": 2**40}
    units = {"K": 2**10, "M": 2**20, "G": 2**30, "T": 2**40}
    size = size.upper()
    #print("parsing size ", size)
    if not re.match(r' ', size):
        # size = re.sub(r'([KMGT]?)[B]', r' \1', size)
        size = re.sub(r'([KMGT]{1})B?', r' \1', size)
    number, unit = [string.strip() for string in size.split()]
    return int(float(number)*units[unit])



if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='')

    parser.add_argument('-f', '--file', type=str, required=True, default=None, help='File to be analyzed')
    parser.add_argument('-p', '--flash', type=parse_size, required=True, help='Max Flash size (in bytes)')
    parser.add_argument('-r', '--ram', type=parse_size, required=True, help='Max RAM size (in bytes)')

    args = parser.parse_args()

    signal.signal(signal.SIGINT, ctrl_c_handler)
    main(args)


