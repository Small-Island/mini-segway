#!/usr/bin/python3
import time
import numpy as np
import serial
import threading
import socket

setVelM1 = 0
setVelM2 = 0
udpRecvTime = 0
run = True

def udp_loop():
    global run, setVelM1, setVelM2, udpRecvTime
    M_SIZE = 1024
    host = '0.0.0.0'
    port = 9001
    sock = socket.socket(socket.AF_INET, type=socket.SOCK_DGRAM)
    sock.settimeout(2)
    print('udp loop: create socket')
    sock.bind((host, port))
    while run:
        try :
            print('udp loop: Waiting message')
            message, cli_addr = sock.recvfrom(M_SIZE)
            udpRecvTime = time.time()
            # print(f'udp loop: Received message is {message}')
            turning_vel = int.from_bytes(message[0:2], byteorder='big', signed=True)
            forward_vel = int.from_bytes(message[2:4], byteorder='big', signed=True)
            print(f'udp loop: velocity ({turning_vel, forward_vel})')
            setVelM1 = forward_vel - turning_vel
            setVelM2 = -(forward_vel + turning_vel)
        except socket.error as e:
            print(f"udp loop: {e}")
        except KeyboardInterrupt :
            run = False
            break
    sock.close()

thread_udp = threading.Thread(target=udp_loop)
thread_udp.start()

import serial
while run:
    try:
        print('serial loop: try to open')
        time.sleep(1)
        ser = serial.Serial('COM6', 9600)
        # ser.close()
    except OSError as e:
        print(f"serial loop: {e}")
        print("serial loop: usb not open. try to open in 1sec")
        continue
    except KeyboardInterrupt :
        run = False
        thread_udp.join()
        # quit()
        break
    except ValueError :
        run = False
        thread_udp.join()
        # quit()
        break
    except serial.SerialException as e:
        print(e)
        run = False
        thread_udp.join()
        # quit()
        break
    except IOError as e:
        print(e)
        run = False
        thread_udp.join()
        # quit()
        break
    else:
        print('else')
        # run = False
        # continue

    print("start")
    while run:
        try:
            if time.time() - udpRecvTime > 1:
                setVelM1 = 0
                setVelM2 = 0   
            time.sleep(0.001)
            # dataBytes = [0, 0, 0, 0, 0]
            # dataBytes[4] = setVelM2 & 0xff
            # dataBytes[3] = (setVelM2 >> 8) & 0xff
            # dataBytes[2] = setVelM1 & 0xff
            # dataBytes[1] = (setVelM1 >> 8) & 0xff
            # ser.write(dataBytes)
            msg = setVelM1.to_bytes(2, 'big', signed=True) + setVelM2.to_bytes(2, 'big', signed=True)
            # msg = b'\x01\xff\x04'
            ser.write(msg)
            print(f"serial loop: send {msg.hex()}")
            x = b''
            for i in range(0, 8):
                x = x + ser.read()
                # print(f"serial loop: read {x}")
            print(f"serial loop: read {x.hex()}")
            print(f"serial loop: targVelM1 {int.from_bytes(x[4:6], byteorder='big', signed=True)}, targVelM2 {int.from_bytes(x[6:8], byteorder='big', signed=True)}")
                    
            # getBytes = ser.read(4)
            # getVelM1 = np.array((getBytes[0] << 8) + getBytes[1], dtype='int16') 
            # getVelM2 = np.array((getBytes[2] << 8) + getBytes[3], dtype='int16')
            # print('setVel', setVelM1, setVelM2, "V_M1" , getVelM1, "V_M2", getVelM2)
            
        except KeyboardInterrupt :
            run = False
            ser.close()
            thread_udp.join()
            break
        except Exception as e:
            print(e)
            print("serial unknown error")
            ser.close()
            break