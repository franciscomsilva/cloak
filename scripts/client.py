import sys, os
import subprocess as sub
import time, sched
import random
import requests
from termcolor import colored
import random
from automate import automateChromium
import signal



def PrintColored(string, color):
    print(colored(string, color))

#################################################################################
# Useful definitions
cloak_bin_folder_location = '/home/vagrant/cloak'
video_folder = "/home/vagrant/Videos"
chromium_builds_folder = "/home/vagrant/chromium_builds/replacement_build"
tor_browser_bin_folder = "/home/vagrant/tor-browser/Browser"
DST_IP = "192.168.56.102"

sync_early = 2
sync_late = 4


#############################################################################
# Choose WebRTC application to test

WEBRTC_APPLICATION = "https://meet.jit.si/cloaktestdasfsgdsa"


#############################################################################

headless_env = dict(os.environ)
headless_env['DISPLAY'] = ':0'

#################################################################################

def RESTCall(method, args=""):
    url='http://' + DST_IP + ':5005/' + method
    response = ''
    try:
        response = requests.post(url, data=args)
    except requests.exceptions.RequestException as e:
        print e

def StartFFMPEGStream(chat_video):
    args = "ffmpeg -nostats -loglevel quiet -re -i " + "\"" + chat_video + "\"" + " -r 30 \
            -vf scale=1280:720 -vcodec rawvideo -pix_fmt yuv420p -threads 0 -f v4l2 /dev/video0"
    sub.Popen(args, shell = True, stdin = open(os.devnull))

def StartFFMPEGStreamLoop(chat_video):
    args = "ffmpeg -nostats -stream_loop -1 -loglevel quiet -re -i " + "\"" + chat_video + "\"" + " -r 30 \
            -vf scale=1280:720 -vcodec rawvideo -pix_fmt yuv420p -threads 0 -f v4l2 /dev/video0"
    sub.Popen(args, shell = True, stdin = open(os.devnull))

def KillFFMPEGStream():
    os.system("pkill -9 -f ffmpeg")


def StartChromium(create_log, chromium_build, webrtc_app):
    args = chromium_builds_folder + "/chrome --disable-session-crashed-bubbles --enable-logging=stderr --v=-3  --vmodule=*/webrtc/*=1 --no-sandbox " + webrtc_app
    if(create_log):
        log = open('chromium_log_sender', 'w')
        sub.Popen(args, env = headless_env, shell = True, stdout=log, stderr=log)
    else:
        devnull = open(os.devnull,'wb')
        sub.Popen(args, env = headless_env, shell = True, stdout=devnull, stderr=devnull)


def KillChromium():
    os.system("pkill -9 -f chrome")

def StartTorBrowser(create_log):
    args = tor_browser_bin_folder + "/start-tor-browser"
    if create_log:
        log = open('torbrowser_log_sender', 'w')
        sub.Popen(args, shell = True, cwd = tor_browser_bin_folder, stdout=log, stderr=log)
    else:
        devnull = open(os.devnull,'wb')
        sub.Popen(args, shell = True, cwd = tor_browser_bin_folder, stdout=devnull, stderr=devnull)
def KillTorBrowser():
    os.system("pkill -9 -f tor")
    os.system("pkill -9 -f firefox")

def StartProcedures(chat_sample, chromium_build):


    print "Chromium build - " + chromium_build

    #Start FFMPEG in sync
    s = sched.scheduler(time.time, time.sleep)
    now = time.time()
    start_remote_ffmpeg = now + sync_early
    start_local_ffmpeg = now + sync_late

    print "[P] Starting remote Looped FFMPEG stream at: " + str(start_remote_ffmpeg)
    RESTCall("startFFMPEGSyncLooped", str(start_remote_ffmpeg) + ",|," + video_folder + "/" + chat_sample)

    print "[P] Wait to start local Looped FFMPEG stream at: " + str(start_local_ffmpeg)
    args = (video_folder + "/" + chat_sample,)
    s.enterabs(start_local_ffmpeg, 0, StartFFMPEGStreamLoop, args)
    s.run()
    print "[P] Starting local FFMPEG stream: " + "/" + chat_sample

    #Start Chromium in sync
    now = time.time()
    start_remote_chromium = now + sync_early
    start_local_chromium = now + sync_late

    sample_index = random.randint(500, 1000)

    webrtc_app = WEBRTC_APPLICATION + str(sample_index)


    print "[P] Wait to start local Chromium Browser at: " + str(start_local_chromium)
    args = (False, chromium_build, webrtc_app)
    s.enterabs(start_local_chromium, 0, StartChromium, args)
    s.run()
    print "[P] Starting local Chromium Browser"

    RESTCall("startTor")
    print "[P] Starting Remote Tor"

    if "jit" in WEBRTC_APPLICATION:
        time.sleep(20)  # Ten seconds were apparently not enough for starting up Chromium

        print "[P] Performing local automation task"
        automateChromium(webrtc_app)
        time.sleep(2)

        print "[P] Starting Remote Chromium Browser at: " + str(start_remote_chromium)
        RESTCall("startChromium", str(start_remote_chromium) + "," + chromium_build + "," + webrtc_app + "," + "nolog")

        time.sleep(20)

        print "[P] Performing remote automation task"
        RESTCall("automateApp", webrtc_app)


        #Wait a few seconds for the WebRTC call to stabilize
        time.sleep(2)

    # Start Tor Browser with cloak
    print "[P] Starting Local Tor Browser"
    StartTorBrowser(False)

    #Wait tor CTRL+C
    while(True):
        continue

    # Cleanup
    print "[P] Killing Tor Browser"
    KillTorBrowser()

    print "[P] Killing FFMPEG stream"
    KillFFMPEGStream()
    print "[P] Killing remote FFMPEG stream"
    RESTCall("killFFMPEG")

    print "[P] Killing Chromium"
    KillChromium(True)

    print "[P] Killing Remote Chromium Browser"
    RESTCall("killChromium")

    print "[P] Killing Remote Tor instance"
    RESTCall("killTor")


def handler(signum, frame):
    # Cleanup
    print "[P] Killing Tor Browser"
    KillTorBrowser()

    print "[P] Killing FFMPEG stream"
    KillFFMPEGStream()
    print "[P] Killing remote FFMPEG stream"
    RESTCall("killFFMPEG")

    print "[P] Killing Chromium"
    KillChromium()

    print "[P] Killing Remote Chromium Browser"
    RESTCall("killChromium")

    print "[P] Killing Remote Tor instance"
    RESTCall("killTor")
    exit(0)

if __name__ == "__main__":
    # Define SIGNAL handler
    signal.signal(signal.SIGINT, handler)

    chromium_builds = ["replacement_build"]

    StartProcedures("gamingchat002.mp4", chromium_builds[0])