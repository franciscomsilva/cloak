import sys, os
import subprocess as sub
import time, sched
import random
import threading
import requests

cloak_folder_location = '/home/vagrant/cloak/src/client/transport/hooks/'
chromium_src_folder = "/home/vagrant/Chromium/chromium/src/"
chromium_builds_folder = "/home/vagrant/chromium_builds/"


# Compile the regular chromium version without hooks
def CompileRegularChromium():
    print("Compiling Regular Chromium")
    cmd = cloak_folder_location + "patch.sh /home/vagrant/Chromium/chromium/src/third_party/webrtc/ hooks/macros macros_regular.h hooks hooks.cpp /home/vagrant/cloak/src/client/transport"
    p = sub.call(cmd, shell=True, cwd=cloak_folder_location)

    my_env = os.environ.copy()
    my_env["PATH"] = my_env["PATH"] + ":/home/vagrant/Chromium/depot_tools"

    # Writes build flags to args.gn
    f = open(chromium_src_folder + "out/quick_build/args.gn", "w")
    f.write("is_debug = false\nis_component_build = true\nsymbol_level = 0\n")
    f.close()

    cmd = "autoninja -C out/quick_build/ chrome"
    p = sub.call(cmd, shell=True, cwd=chromium_src_folder, env=my_env)

    cmd = "cp -v -T -r out/quick_build/ /home/vagrant/chromium_builds/regular_build"
    p = sub.call(cmd, shell=True, cwd=chromium_src_folder, env=my_env)


# Compile hook-powered Chromium browser
def CompileFullFrameReplacementChromium():
    print("Compiling Full Frame Replacement Chromium")
    cmd = cloak_folder_location + "patch.sh /home/vagrant/Chromium/chromium/src/third_party/webrtc/ hooks/macros macros_replacement.h hooks hooks.cpp /home/vagrant/cloak/src/client/transport"
    p = sub.call(cmd, shell=True, cwd=cloak_folder_location)

    my_env = os.environ.copy()
    my_env["PATH"] = my_env["PATH"] + ":/home/vagrant/Chromium/depot_tools"

    # Writes build flags to args.gn
    f = open(chromium_src_folder + "out/quick_build/args.gn", "w")
    f.write("is_debug = false\nis_component_build = true\nsymbol_level = 0\nrtc_libvpx_build_vp9 = false\n")
    f.close()

    cmd = "autoninja -C out/quick_build/ chrome"
    p = sub.call(cmd, shell=True, cwd=chromium_src_folder, env=my_env)

    cmd = "cp -v -T -r out/quick_build/ /home/vagrant/chromium_builds/ReplacementFullFrame_build"
    p = sub.call(cmd, shell=True, cwd=chromium_src_folder, env=my_env)


def CompileChromiumVersions(chromium_builds_folder):
    if not os.path.exists(chromium_builds_folder):
        os.makedirs(chromium_builds_folder)

    #CompileRegularChromium()
    CompileFullFrameReplacementChromium()


if __name__ == "__main__":
    CompileChromiumVersions(chromium_builds_folder)