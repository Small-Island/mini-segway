#!/usr/bin/python3
import time
import numpy as np
import serial
import threading
import socket

setVelM1 = np.array(0, dtype='int16')
setVelM2 = np.array(0, dtype='int16')
momoRecvTime = 0
run = True

#turning_vel = 0.0 # -45 ~ 45 (deg/sec)
#forward_vel = 0.0 # -1.0 ~ 1.0 (meter/sec)

def udp_loop():
    global run, setVelM1, setVelM2, udpRecvTime
    M_SIZE = 16
    host = "0.0.0.0"
    port = 9001
    sock = socket.socket(socket.AF_INET, type=socket.SOCK_DGRAM)
    sock.settimeout(1)
    print('create socket')
    sock.bind((host, port))
    # global setVelM1, setVelM2, momoRecvTime
    # ser = serial.Serial('./serial_out', 9600)
    while run:
        try :
            print('udp loop: Waiting message')
            message, cli_addr = sock.recvfrom(M_SIZE)
            udpRecvTime = time.time()
            print(f'udp loop: Received message is [{message}]')
            np_array = np.array([message[0], message[1]], dtype='int8')
            turning_vel = int(np_array[0])/127 # -127 ~ 127
            forward_vel = int(np_array[1])/127 # -127 ~ 127
            setVelM1_int32 = 150 * forward_vel - 50*turning_vel
            setVelM2_int32 = -(150 * forward_vel + 50*turning_vel)
            setVelM1 = np.array(setVelM1_int32, dtype='int16')
            setVelM2 = np.array(setVelM2_int32, dtype='int16')
        except socket.error as e:
            print(f"udp loop: {e}")
        except KeyboardInterrupt :
            run = False
            break
    sock.close()
        # print('recvVel', setVelM1, setVelM2)
        # print("hello")
        # data = ser.read(4)
        # momoRecvTime = time.time()
        # read_array = np.array([data[0], data[1], data[2], data[3]], dtype='int8')
        # turning_vel = int(read_array[2])/127 # -127 ~ 127
        # forward_vel = int(read_array[3])/127 # -127 ~ 127
        
        # ######################################
        # setVelM1_int32 = 150 * forward_vel - 50*turning_vel
        # setVelM2_int32 = -(150 * forward_vel + 50*turning_vel)
        # #######################################
        
        # setVelM1 = np.array(setVelM1_int32, dtype='int16')
        # setVelM2 = np.array(setVelM2_int32, dtype='int16')
        
        # print('recvVel', setVelM1, setVelM2)

thread_udp = threading.Thread(target=udp_loop)
thread_udp.start()

import hid
h = hid.device()
while run:
    try:
        time.sleep(1)
        h.open(0x2886, 0x802f)
    except OSError as e:
        print(f"hid loop: {e}")
        print("hid loop: usb not open. try to open in 1sec")
        continue
    except KeyboardInterrupt :
        run = False
        thread_udp.join()
        # quit()
        break
    else:
        break

    print("start")
    while run:
        try:
            if time.time() - momoRecvTime > 1:
                setVelM1 = 0
                setVelM2 = 0   
            time.sleep(0.05)
            dataBytes = [0, 0, 0, 0, 0]
            dataBytes[4] = setVelM2 & 0xff
            dataBytes[3] = (setVelM2 >> 8) & 0xff
            dataBytes[2] = setVelM1 & 0xff
            dataBytes[1] = (setVelM1 >> 8) & 0xff
            h.write(dataBytes)
            getBytes = h.read(4)
            getVelM1 = np.array((getBytes[0] << 8) + getBytes[1], dtype='int16') 
            getVelM2 = np.array((getBytes[2] << 8) + getBytes[3], dtype='int16')
            print('setVel', setVelM1, setVelM2, "V_M1" , getVelM1, "V_M2", getVelM2)
            
        except KeyboardInterrupt :
            run = False
            h.close()
            thread_udp.join()
            break
        except Exception as e:
            print(e)
            print("hid unknown error")
            h.close()