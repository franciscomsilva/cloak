/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef MODULES_VIDEO_CODING_ENCODED_FRAME_H_
#define MODULES_VIDEO_CODING_ENCODED_FRAME_H_

#include <vector>

#include "api/video/encoded_image.h"
#include "modules/rtp_rtcp/source/rtp_video_header.h"
#include "modules/video_coding/include/video_codec_interface.h"
#include "modules/video_coding/include/video_coding_defines.h"
#include "rtc_base/system/rtc_export.h"


namespace webrtc {

    class RTC_EXPORT VCMEncodedFrame : public EncodedImage {
    public:
    VCMEncodedFrame();
    VCMEncodedFrame(const VCMEncodedFrame&);
    ~VCMEncodedFrame();
    /**
     *   Set render time in milliseconds
     */
    void SetRenderTime(const int64_t renderTimeMs) {
        _renderTimeMs = renderTimeMs;
    }
    VideoPlayoutDelay PlayoutDelay() const { return playout_delay_; }
    void SetPlayoutDelay(VideoPlayoutDelay playout_delay) {
        playout_delay_ = playout_delay;
    }
    /**
     *   Get the encoded image
     */
    const webrtc::EncodedImage& EncodedImage() const {
        return static_cast<const webrtc::EncodedImage&>(*this);
    }
    using EncodedImage::ColorSpace;
    using EncodedImage::data;
    using EncodedImage::GetEncodedData;
    using EncodedImage::NtpTimeMs;
    using EncodedImage::PacketInfos;
    using EncodedImage::set_size;
    using EncodedImage::SetColorSpace;
    using EncodedImage::SetEncodedData;
    using EncodedImage::SetPacketInfos;
    using EncodedImage::SetSpatialIndex;
    using EncodedImage::SetSpatialLayerFrameSize;
    using EncodedImage::SetTimestamp;
    using EncodedImage::size;
    using EncodedImage::SpatialIndex;
    using EncodedImage::SpatialLayerFrameSize;
    using EncodedImage::Timestamp;

    //PROTOZOA
    /***********************************************************/
    uint8_t* GetMutableBuffer() { return mutable_data(); }

    void SetMutableBuffer(uint8_t* buffer, int buffer_len) {
        memcpy(mutable_data(), buffer, buffer_len);
    }

    size_t GetLength() const { return size(); }

    void SetLength(size_t len) { set_size(len); }

    size_t GetSize() const { return size(); }

    void SetSize(size_t size) { set_size(size); }

    /***********************************************************/


    /**
     *   Get render time in milliseconds
     */
    int64_t RenderTimeMs() const { return _renderTimeMs; }
    /**
     *   Get frame type
     */
    webrtc::VideoFrameType FrameType() const { return _frameType; }
    /**
     *   Set frame type
     */
    void SetFrameType(webrtc::VideoFrameType frame_type) {
        _frameType = frame_type;
    }
    /**
     *   Get frame rotation
     */
    VideoRotation rotation() const { return rotation_; }
    /**
     *  Get video content type
     */
    VideoContentType contentType() const { return content_type_; }
    /**
     * Get video timing
     */
    EncodedImage::Timing video_timing() const { return timing_; }
    EncodedImage::Timing* video_timing_mutable() { return &timing_; }
    /**
     *   True if there's a frame missing before this frame
     */
    bool MissingFrame() const { return _missingFrame; }
    /**
     *   Payload type of the encoded payload
     */
    uint8_t PayloadType() const { return _payloadType; }
    /**
     *   Get codec specific info.
     *   The returned pointer is only valid as long as the VCMEncodedFrame
     *   is valid. Also, VCMEncodedFrame owns the pointer and will delete
     *   the object.
     */
    const CodecSpecificInfo* CodecSpecific() const { return &_codecSpecificInfo; }
    void SetCodecSpecific(const CodecSpecificInfo* codec_specific) {
        _codecSpecificInfo = *codec_specific;
    }

    webrtc::VideoCodecType GetCodec(){
        return _codec;
    }


    protected:
    void Reset();
    void CopyCodecSpecific(const RTPVideoHeader* header);
    int64_t _renderTimeMs;
    uint8_t _payloadType;
    bool _missingFrame;
    CodecSpecificInfo _codecSpecificInfo;
    webrtc::VideoCodecType _codec;
};

}  // namespace webrtc

#endif  // MODULES_VIDEO_CODING_ENCODED_FRAME_H_