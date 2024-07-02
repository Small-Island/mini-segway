import sys, os

# os.environ["SDL_VIDEODRIVER"] = "dummy"
# os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = "hide"

import pygame
import numpy as np
import time

turning_vel = 0
forward_vel = 0

def joy_event(joy):
    global turning_vel, forward_vel
    # axis = (joy.get_axis(0), joy.get_axis(1)) # 左スティックの左右．
    # print(f"Get status of left axis (左右，上下) = {axis}")
    axis = (joy.get_axis(2), joy.get_axis(3)) # 右スティックの左右．
    # print(f"Get status of right axis (左右，上下) = {axis}")
    turning_vel = int(axis[0]*100)
    forward_vel = int(axis[1]*100)

run = True

def udp_loop():
    global run, turning_vel, forward_vel
    import socket
    M_SIZE = 1024
    host = 'localhost'
    port = 9001
    sock = socket.socket(socket.AF_INET, type=socket.SOCK_DGRAM)
    print('create socket')
    while run:
        try :
            print("To quit, press B-button.")
            print(f'udp loop: sending ({turning_vel, forward_vel})')
            msg = turning_vel.to_bytes(2, 'big', signed=True) + forward_vel.to_bytes(2, 'big', signed=True)
            print(f'udp loop: sending {msg}')
            sock.sendto(msg, (host, port))
            time.sleep(0.2)
        except socket.error as e:
            print(f"udp loop: {e}")
        except KeyboardInterrupt :
            run = False
            break
    sock.close()

import threading      
thread_udp = threading.Thread(target=udp_loop)
thread_udp.start()    

def main():
    global run
    pygame.init()

    try:
        joy = pygame.joystick.Joystick(0)
        joy.init()
        print('Joystick Name: ', joy.get_name())
        print('Number of buttons: ', joy.get_numbuttons())

    except pygame.error:
        print('Joystick 0 is not connected.')
        sys.exit(1)

    try:
        while run:
            event = pygame.event.wait() # Wait until getting event from queue

            btn_b = joy.get_button(1) # B button to exit
            if btn_b == 1:
                run = False
                break

            # print("To quit, press B-button.")
            joy_event(joy)

            #print(event)

    except KeyboardInterrupt:
        print("Program interrupted by user")

    joy.quit()
    pygame.quit()

    print("Exit")

    return

if __name__=="__main__":
    main()
