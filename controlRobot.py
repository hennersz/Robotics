#!/usr/bin/env python

import time
import socket
import sys
import tty
import termios

#Allows you to control robot by using arrow keys or WASD
#in order to increase or decrease speed, use '>' and '<'
#press '0' if you want to make the robot stop immediately.
#' ' will cause the program to terminate

UP = 0
DOWN = 1
RIGHT = 2
LEFT = 3

TCP_IP = '128.16.79.6'
TCP_PORT = 55443
BUFFER_SIZE = 1024

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))

def readchar():
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(sys.stdin.fileno())
        ch = sys.stdin.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    if ch == '0x03':
        raise KeyboardInterrupt
    return ch

def readkey(getchar_fn=None):
    getchar = getchar_fn or readchar
    c1 = getchar()
    if ord(c1) != 0x1b:
        return c1
    c2 = getchar()
    if ord(c2) != 0x5b:
        return c1
    c3 = getchar()
    return ord(c3) - 65  # 0=Up, 1=Down, 2=Right, 3=Left arrows

speed = 30

try:
    while True:
        keyp = readkey()
        if keyp == 'w' or keyp == UP:
            s.send("M LR {0} {1}\n".format(speed, speed))
            data = s.recv(BUFFER_SIZE)
            sys.stdout.flush()
            print 'Forward', speed
        elif keyp == 's' or keyp == DOWN:
            s.send("M LR {0} {1}\n".format(-speed, -speed))
            data = s.recv(BUFFER_SIZE)
            sys.stdout.flush()
            print 'Backward', speed
        elif keyp == 'd' or keyp == RIGHT:
            s.send("M LR {0} {1}\n".format(speed/2, -speed/2))
            data = s.recv(BUFFER_SIZE)
            sys.stdout.flush()
            print 'Spin Right', speed
        elif keyp == 'a' or keyp == LEFT:
            s.send("M LR {0} {1}\n".format(-speed/2, speed/2))
            data = s.recv(BUFFER_SIZE)
            sys.stdout.flush()            
            print 'Spin Left', speed
        #elif keyp == UP and keyp == LEFT:
        #    temp = speed/10
        #    s.send("M LR {0} {1}".format(speed + temp, speed - temp))
        #    data = s.recv(BUFFER_SIZE)            
        #    print 'Left Forward', speed 
        #elif keyp == UP and keyp == RIGHT:
        #    temp = speed/10
        #    s.send("M LR {0} {1}".format(speed - temp, speed + temp))
        #    data = s.recv(BUFFER_SIZE)            
        #    print 'Left Forward', speed        
        elif keyp == '.' or keyp == '>':
            speed = min(127, speed+10)
            sys.stdout.flush()
            print 'Speed+', speed
        elif keyp == ',' or keyp == '<':
            speed = max (0, speed-10)
            sys.stdout.flush()
            print 'Speed-', speed
        elif keyp == '0':
            s.send("M LR 0 0\n")
            data = s.recv(BUFFER_SIZE)  
            sys.stdout.flush()
        elif keyp == ' ':
            s.close()
            print 'Stop'
        elif ord(keyp) == 3:
            break
        else:
            s.send("M LR 0 0\n")
            data = s.recv(BUFFER_SIZE)
            sys.stdout.flush()  
except KeyboardInterrupt:
    s.close()