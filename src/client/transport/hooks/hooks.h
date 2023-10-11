#ifndef HOOKS_H_
#define HOOKS_H_

#include <string.h>
#include <string>
#include <mutex>
#include <map>
#include "debug.h"
#include "macros.h"
#include "../api/video/video_codec_type.h"


void printFrameData(uint8_t* frame_buffer, int frame_length);

void printEncodedImageInfo(uint32_t encodedWidth, uint32_t encodedHeight, size_t length, size_t size, bool completeFrame, int qp, int frameType, uint8_t* buffer, webrtc::VideoCodecType codec, bool printBuffer);

void printEncodedImageInfo(uint32_t encodedWidth, uint32_t encodedHeight, size_t length, size_t size, bool completeFrame, int qp, int frameType, uint8_t* buffer, bool printBuffer);

//void printResolutionDebugInfo(std::string prefix, std::map<std::string, int> resolution_counter, std::map<std::string, DebugTorcloak::ResolutionChange> resolution_change_tracker);

void encodeDataIntoFrame(uint8_t* frame_buffer, int frame_length, int dct_partition_offset, int frame_type);


void retrieveDataFromFrame(uint8_t* frame_buffer, int frame_length, int dct_partition_offset, int frame_type) ;
void probePrint(std::string msg);

void readFromPipe();

static inline uint32_t GetBitsAt(uint32_t data, size_t shift, size_t num_bits) {
  return ((data >> shift) & ((1 << num_bits) - 1));
}

#endif //HOOKS_H_
