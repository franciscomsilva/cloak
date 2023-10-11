"""
This file deals with GUI automation to test several WebRTC apps

Screen resolution should be set to 1280x768

Chromium browser window should be set to maximize
"""
import os
os.environ['DISPLAY'] = ':0'

import pyautogui
import sys
import time


pyautogui.PAUSE = 0.25
pyautogui.FAILSAFE = True

def JitsiAutomation():
    time.sleep(0.25)
    join_button = (201, 448)
    pyautogui.click(join_button[0], join_button[1], button='left')

def automateChromium(webrtc_application):
    if ("jit" in webrtc_application):
        print "Jitsi Meet Automation: Started"
        JitsiAutomation()

def gracefullyCloseChromium():
    pyautogui.click(1265, 20, button='left')
    pyautogui.click(1265, 20, button='left')



if __name__ == "__main__":
    if(len(sys.argv) < 2):
        print "Input intended application"
        sys.exit(0)

    webrtc_application = sys.argv[1]
    automateChromium(webrtc_application, "callee")