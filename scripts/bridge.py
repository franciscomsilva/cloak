import sys, os
import subprocess as sub
import time, sched
from flask import Flask
from flask import request
from termcolor import colored 
from automate import automateChromium, gracefullyCloseChromium


def PrintColored(string, color):
    print(colored(string, color))

#################################################################################
# Useful definitions

cloak_bin_folder_location = '/home/vagrant/cloak'
chromium_builds_folder = "/home/vagrant/chromium_builds/replacement_build"

headless_env = dict(os.environ)
headless_env['DISPLAY'] = ':0'

NETWORK_INTERFACE = "enp0s8"
NETWORK_INTERFACE_FOR_OPEN_SERVER = "enp0s9"
#################################################################################

app = Flask(__name__)

def startFFMPEGAtClockLooped(video_sample):
    #If no argument is passed in sender control, use default video
    if(len(video_sample) == 0):
        video_sample = "/home/vagrant/SharedFolder/<some-video>.mp4"
    else:
        #Else, mirror video content at the sender
        video_sample = video_sample

    PrintColored("Starting FFMPEG stream - " + video_sample, 'red')

    args = "ffmpeg -nostats -stream_loop -1 -loglevel quiet -re -i " + video_sample + " -r 30 \
            -vf scale=1280:720 -vcodec rawvideo -pix_fmt yuv420p -threads 0 -f v4l2 /dev/video0"
    sub.Popen(args, shell = True, stdin = open(os.devnull))


@app.route('/startFFMPEGSyncLooped', methods=['POST'])
def startFFMPEGSyncLooped():
    time_sync, video_sample = request.data.split(",|,")
    s = sched.scheduler(time.time, time.sleep)
    print "Time now: " + str(time.time())
    print "Time sync: " + time_sync
    args = (video_sample, )
    s.enterabs(float(time_sync), 0, startFFMPEGAtClockLooped, args)
    s.run()
    return "Starting FFMPEG stream Looped"

@app.route('/killFFMPEG', methods=['POST'])
def killFFMPEG():
    PrintColored("Killing FFMPEG stream", 'red')
    os.system("pkill -9 -f ffmpeg")
    return "Killing FFMPEG stream"


def startChromiumSync(chromium_build, webrtc_application, log):
    PrintColored("Starting Chromium - " + chromium_build, 'red')
    args = chromium_builds_folder  + "/chrome --disable-session-crashed-bubbles --enable-logging=stderr --v=-3 --vmodule=*/webrtc/*=1 --no-sandbox " + webrtc_application

    if log == "log":
        log = open("chromium_log_receiver",'wb')
        sub.Popen(args, env=headless_env, shell = True, stdout=log, stderr=log)
    else:
        devnull = open(os.devnull, 'wb')
        sub.Popen(args, env=headless_env, shell=True, stdout=devnull, stderr=devnull)

@app.route('/startChromium', methods=['POST'])
def startChromium():
    timing, chromium_build, webrtc_application, log = request.data.split(",")
    s = sched.scheduler(time.time, time.sleep)
    print "Time now: " + str(time.time())
    print "Time sync: " + request.data
    args = (chromium_build, webrtc_application, log, )
    s.enterabs(float(timing), 0, startChromiumSync, args)
    s.run()
    return "Starting Chromium - " + chromium_build

def killProxy():
    os.system("pkill -9 -f \"ssh -i\"")

@app.route('/killChromium', methods=['POST'])
def killChromium():
    PrintColored("Killing Chromium", 'red')
    os.system("pkill -9 -f chrome")

    killProxy()
    return "Killing Chromium"

@app.route('/automateApp', methods=['POST'])
def automateApp():
    PrintColored("Automating Chromium", 'red')
    webrtc_application = request.data
    automateChromium(webrtc_application)
    return "Automated Chromium"

@app.route('/gracefullyCloseChromium', methods=['POST'])
def gracefullyCloseChromium():
    PrintColored("Gracefully closing Chromium", 'red')
    gracefullyCloseChromium()
    return "Gracefully closing Chromium"


@app.route('/startTor', methods=['POST'])
def startTor():
    PrintColored("Starting Tor", 'red')
    args = "tor -f " + cloak_bin_folder_location + "/tor_confs/torrc_bridge_managed"
    devnull = open(os.devnull, 'wb')
    sub.Popen(args, shell = True, stdout=devnull, stderr=devnull)
    return "Starting Tor"

@app.route('/killTor', methods=['POST'])
def killTor():
    PrintColored("Killing Tor", 'red')
    os.system("pkill -9 -f tor")
    return "Killing Tor"

if __name__ == "__main__":
    app.run(debug=False, host='0.0.0.0',port=5005)
