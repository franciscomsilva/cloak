#!/bin/bash

##############################################################################
# Patch files within Chromium's WebRTC stack
# This script will be executed from Cloak/machine_setup/compile_chromium.py
##############################################################################

CHROMIUM_WEBRTC_DIR=$1
MACRO_DIR=$2
MACRO_FILE=$3
HOOK_DIR=$4
HOOK_FILE=$5
CLOAK_DIR=$6

#Copy hooks and code to Chromium's WebRTC directory
rm -r $CHROMIUM_WEBRTC_DIR/hooks
mkdir $CHROMIUM_WEBRTC_DIR/hooks

cp $CLOAK_DIR/$HOOK_DIR/BUILD.gn                                                               $CHROMIUM_WEBRTC_DIR/hooks/BUILD.gn
cp $CLOAK_DIR/$HOOK_DIR/hooks.h                                                      $CHROMIUM_WEBRTC_DIR/hooks/hooks.h
cp $CLOAK_DIR/$HOOK_DIR/debug.h                                                      $CHROMIUM_WEBRTC_DIR/hooks/debug.h
cp $CLOAK_DIR/$HOOK_DIR/recorded_frames/recorded_frames.h                   $CHROMIUM_WEBRTC_DIR/hooks/recorded_frames.h


cp $CLOAK_DIR/$HOOK_DIR/$HOOK_FILE                                                   $CHROMIUM_WEBRTC_DIR/hooks/hooks.cpp
cp $CLOAK_DIR/$MACRO_DIR/$MACRO_FILE                                                 $CHROMIUM_WEBRTC_DIR/hooks/macros.h

#Include Protozoa in Chromium's linking step
cp $CLOAK_DIR/chromium_patches/media/BUILD.gn 										                          $CHROMIUM_WEBRTC_DIR/media/BUILD.gn

#Overwrite Webrtc code to apply our hooks
#Global
cp $CLOAK_DIR/chromium_patches/api/video/encoded_image.h 						                      $CHROMIUM_WEBRTC_DIR/api/video/encoded_image.h
cp $CLOAK_DIR/chromium_patches/modules/video_coding/encoded_frame.h 						            $CHROMIUM_WEBRTC_DIR/modules/video_coding/encoded_frame.h


#Sender
cp $CLOAK_DIR/chromium_patches/call/rtp_video_sender.cc 							                      $CHROMIUM_WEBRTC_DIR/call/rtp_video_sender.cc
cp $CLOAK_DIR/chromium_patches/call/rtp_video_sender.h							                        $CHROMIUM_WEBRTC_DIR/call/rtp_video_sender.h
cp $CLOAK_DIR/chromium_patches/video/video_stream_encoder.h                                $CHROMIUM_WEBRTC_DIR/video/video_stream_encoder.h
cp $CLOAK_DIR/chromium_patches/video/video_stream_encoder.cc                               $CHROMIUM_WEBRTC_DIR/video/video_stream_encoder.cc
cp $CLOAK_DIR/chromium_patches/modules/pacing/pacing_controller.h                          $CHROMIUM_WEBRTC_DIR/modules/pacing/pacing_controller.h
cp $CLOAK_DIR/chromium_patches/modules/pacing/pacing_controller.cc                         $CHROMIUM_WEBRTC_DIR/modules/pacing/pacing_controller.cc


#Receiver
cp $CLOAK_DIR/chromium_patches/video/video_receive_stream2.cc 			                        $CHROMIUM_WEBRTC_DIR/video/video_receive_stream2.cc
cp $CLOAK_DIR/chromium_patches/video/video_receive_stream2.h			                          $CHROMIUM_WEBRTC_DIR/video/video_receive_stream2.h
cp $CLOAK_DIR/chromium_patches/video/rtp_video_stream_receiver2.cc                         $CHROMIUM_WEBRTC_DIR/video/rtp_video_stream_receiver2.cc
cp $CLOAK_DIR/chromium_patches/video/rtp_video_stream_receiver2.h                          $CHROMIUM_WEBRTC_DIR/video/rtp_video_stream_receiver2.h
cp $CLOAK_DIR/chromium_patches/modules/video_coding/frame_buffer2.h                        $CHROMIUM_WEBRTC_DIR/modules/video_coding/frame_buffer2.h
cp $CLOAK_DIR/chromium_patches/modules/video_coding/frame_buffer2.cc                       $CHROMIUM_WEBRTC_DIR/modules/video_coding/frame_buffer2.cc
cp $CLOAK_DIR/chromium_patches/call/call.h 							                                  $CHROMIUM_WEBRTC_DIR/call/call.h
cp $CLOAK_DIR/chromium_patches/call/call.cc 							                                  $CHROMIUM_WEBRTC_DIR/call/call.cc


