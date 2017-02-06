import argparse
import os

def main():
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('--line', action='store', )
    parser.add_argument('--fifo_file', action='store')
    args = parser.parse_args()
    new_line = args.line
    fifo_file = args.fifo_file
    new_line = new_line.replace("\\t", '\t').replace("\\n", "\n")
    new_line = new_line.encode()
    file_handler = os.fdopen(os.open(fifo_file, os.O_WRONLY | os.O_NONBLOCK), 'wb')
    file_handler.write(new_line)
    print("OK)")

if __name__ == '__main__':
    main()
