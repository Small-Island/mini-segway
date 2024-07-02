import sys, os

# os.environ["SDL_VIDEODRIVER"] = "dummy"
# os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = "hide"

import pygame

def joy_event(joy):

    hat = joy.get_hat(0) # Get status of cross key

    print(f"Get status of cross key {hat}")
    axis = (joy.get_axis(0), joy.get_axis(1)) # 左スティックの左右．-1が左
    print(f"Get status of left axis (左右，上下) = {axis}")
    axis = (joy.get_axis(2), joy.get_axis(3)) # 左スティックの左右．-1が左
    print(f"Get status of right axis (左右，上下) = {axis}")
    # if hat[0] == 0 and hat[1] == 1: # Up key pressed
    #     print("up")
    # elif hat[0] == 0 and hat[1] == -1: # Down key pressed
    #    print("down")
    # elif hat[0] == -1 and hat[1] == 0: # Left key pressed
    #    print("left")
    # elif hat[0] == 1 and hat[1] == 0: # Right key pressed
    #    print("right")


def main():

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
        while True:
            event = pygame.event.wait() # Wait until getting event from queue

            btn_b = joy.get_button(1) # B button to exit
            if btn_b == 1:
                break

            print("To quit, press B-button.")
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
