// baseds on ffmpeg 6 release
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <functional>
extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavutil/opt.h"
}
#include "color_log.hpp"
#include "image_tools.hpp"
char av_error[AV_ERROR_MAX_STRING_SIZE] = { 0 };
#define av_err2str(errnum) av_make_error_string(av_error, AV_ERROR_MAX_STRING_SIZE, errnum)
using image_process_type = std::function<void(const unsigned char *, const unsigned char *, const unsigned char *)>;
class ffmpeg_mp4_code_decode {
private:
    const AVCodec* codec_ = nullptr;
    AVCodecContext* codec_ctx_ = nullptr;
    AVCodecParserContext *parser_ = nullptr;
    AVFrame* av_frame_ = nullptr;
    AVPacket* packet_ = nullptr;
    int fps_ = 30;
    int width_ = 1920;
    int height_ = 1080;
    const char* code_mp4_path_ = "./tmp.mp4";
    FILE *file_ = nullptr;
    image_process_type image_processor_;
    uint8_t *decode_buf = nullptr;
public:
    ffmpeg_mp4_code_decode() = default;
    virtual ~ffmpeg_mp4_code_decode() {
        uninit_resource();
        if (decode_buf != nullptr) {
            free(decode_buf);
            decode_buf = nullptr;
        }
    }
    inline void set_fps(int fps) {
        fps_ = fps;
    }
    inline void set_scale(int w, int h) {
        width_ = w;
        height_ = h;
    }
    bool init_code() {
        codec_ = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
        if (nullptr == codec_) {
            LOG_E("codec for mpeg4 is null!");
            return false;
	    }
        codec_ctx_ = avcodec_alloc_context3(codec_);
        if (nullptr == codec_ctx_) {
            LOG_E("could not allocate video codec context!");
            return false;
    	}
        codec_ctx_->bit_rate = 5200000;
        codec_ctx_->width = width_;
        codec_ctx_->height = height_;
        codec_ctx_->time_base = (AVRational){1, fps_};
        codec_ctx_->framerate = (AVRational){fps_, 1};
        codec_ctx_->gop_size = 10;
        codec_ctx_->max_b_frames = 1;
        codec_ctx_->pix_fmt = AV_PIX_FMT_YUV420P;
        int ret = avcodec_open2(codec_ctx_, codec_, nullptr);
        if (ret < 0) {
            LOG_E("could not open codec, error code:%d, error string:%s", ret, av_err2str(ret));
            return false;
    	}
        packet_ = av_packet_alloc();
        if (nullptr == packet_) {
            LOG_E("allocate for packet failed!");
            return false;
        }
        av_frame_ = av_frame_alloc();
        if (nullptr == av_frame_) {
            LOG_E("allocate for video frame failed!");
            return false;
    	}
        av_frame_->format = codec_ctx_->pix_fmt;
        av_frame_->width  = codec_ctx_->width;
        av_frame_->height = codec_ctx_->height;
        ret = av_frame_get_buffer(av_frame_, 0);
        if (ret < 0) {
            LOG_E("could not allocate the video frame data, error code:%d, error string:%s", ret, av_err2str(ret));
            return false;
   	    }
        file_ = fopen(code_mp4_path_, "wb");
        if (nullptr == file_) {
            LOG_E("%s open failed!", code_mp4_path_);
            return false;
        }
        LOG_I("code for mp4 init success!");
        return true;
    }
    bool init_decode() {
        codec_ = avcodec_find_decoder(AV_CODEC_ID_MPEG4);
        if (nullptr == codec_) {
            LOG_E("decodec for mpeg4 is null!");
            return false;
	    }
        parser_ = av_parser_init(codec_->id);
        if (nullptr == parser_) {
            LOG_E("parser for MP4 not found!");
            return false;
        }
        codec_ctx_ = avcodec_alloc_context3(codec_);
        if (nullptr == codec_ctx_) {
            LOG_E("could not allocate video codec context!");
            return false;
    	}
        codec_ctx_->width = width_;
        codec_ctx_->height = height_;
        int ret = avcodec_open2(codec_ctx_, codec_, nullptr);
        if (ret < 0) {
            LOG_E("could not open codec, error code:%d, error string:%s", ret, av_err2str(ret));
            return false;
    	}
        packet_ = av_packet_alloc();
        if (nullptr == packet_) {
            LOG_E("allocate for packet failed!");
            return false;
        }
        av_frame_ = av_frame_alloc();
        if (nullptr == av_frame_) {
            LOG_E("allocate for video frame failed!");
            return false;
    	}
        file_ = fopen(code_mp4_path_, "rb");
        if (nullptr == file_) {
            LOG_E("%s open failed!", code_mp4_path_);
            return false;
        }
        size_t size = sizeof(uint8_t) * width_ * height_ * 3 / 2 + AV_INPUT_BUFFER_PADDING_SIZE;
        decode_buf = (uint8_t*)malloc(size);
        memset(decode_buf, 0, size);
        LOG_I("decode for mp4 init success!");
        return true;
    }
    inline void set_code_mp4_path(const char* path) {
        code_mp4_path_ = path;
    }
    void code_i420_frame(unsigned char* frame_y, 
                         unsigned char* frame_u,
                         unsigned char* frame_v) {
        static int64_t pts = 0;
        int ret = av_frame_make_writable(av_frame_);
        if (ret < 0) {
            LOG_E("make frame be writable failed, error code:%d, error string:%s", ret, av_err2str(ret));
            return;
        }
        av_frame_->data[0] = frame_y;
        av_frame_->data[1] = frame_u;
        av_frame_->data[2] = frame_v;
        av_frame_->pts = ++pts;
        encode(codec_ctx_, av_frame_, packet_, file_);
    }
    void get_code_mp4() {
        encode(codec_ctx_, nullptr, packet_, file_);
        fclose(file_);
        file_ = nullptr;
    }
    inline void set_image_processor(const image_process_type &processor) {
        image_processor_ = processor;
    }
    void get_decode_i420_frame() {
        uint8_t *data = nullptr;
        size_t data_size = 0;
        int eof = 0;
        int buf_size = width_ * height_ * 3 / 2;
        int ret = 0;
        do {
            data_size = fread(decode_buf, 1, buf_size, file_);  // read size
            if (ferror(file_)) {
                LOG_E("read file:%s error:%s", code_mp4_path_, strerror(errno));
                break;
            }
            eof = !data_size;
            data = decode_buf;
            while (data_size > 0) {
                ret = av_parser_parse2(parser_, codec_ctx_, &packet_->data, &packet_->size,
                                       data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
                if (ret < 0) {
                    LOG_E("error while parsing, error code:%d, error string:%s", ret, av_err2str(ret));
                    return;
                }
                data += ret;
                data_size -= ret;
                if (packet_->size > 0) {
                    decode(codec_ctx_, av_frame_, packet_);
                }
            } 
        } while (!eof);
        decode(codec_ctx_, av_frame_, nullptr);
    }
private:
    void uninit_resource() {
        if (codec_ctx_ != nullptr) {
            avcodec_free_context(&codec_ctx_);
            codec_ctx_ = nullptr;
        }
        if (packet_ != nullptr) {
            av_packet_free(&packet_);
            packet_ = nullptr;
        }
        if (av_frame_ != nullptr) {
            av_frame_free(&av_frame_);
            av_frame_ = nullptr;
        }
        if (file_ != nullptr) {
            fclose(file_);
            file_ = nullptr;
        }
        if (parser_ != nullptr) {
            av_parser_close(parser_);
            parser_ = nullptr;
        }
    }
    void encode(AVCodecContext *enc_ctx,
                AVFrame *frame,
                AVPacket *pkt,
                FILE *outfile) {
        // send the frame to the encoder
        int ret = avcodec_send_frame(enc_ctx, frame);
        if (ret < 0) {
            LOG_E("error sending a frame for encoding, error code:%d, error string:%s", ret, av_err2str(ret));
            return;
        }
        while (ret >= 0) {
            ret = avcodec_receive_packet(enc_ctx, pkt);
            if ((AVERROR(EAGAIN) == ret) || (AVERROR_EOF == ret)) {
                return;
            }
            else if (ret < 0) {
                LOG_E("error during encoding, error code:%d, error string:%s", ret, av_err2str(ret));
                return;
            }
            fwrite(pkt->data, pkt->size, 1, outfile);
            av_packet_unref(pkt);
        }
    }
    void decode(AVCodecContext *dec_ctx,
                AVFrame *frame,
                AVPacket *pkt) {
        int ret = avcodec_send_packet(dec_ctx, pkt);
        if (ret < 0) {
            LOG_E("error sending a packet for decoding, error code:%d, error string:%s", ret, av_err2str(ret));
            return;
        }
        while (ret >= 0) {
            ret = avcodec_receive_frame(dec_ctx, frame);
            if ((AVERROR(EAGAIN) == ret) || (AVERROR_EOF == ret)) {
                return;
            }
            else if (ret < 0) {
                LOG_E("error during decoding, error code:%d, error string:%s", ret, av_err2str(ret));
                return;
            }
            LOG_D("get frame y address:%p, size:%d, width:%d, height:%d", frame->data[0], frame->linesize[0], frame->width, frame->height);
            LOG_D("get frame u address:%p, size:%d, width:%d, height:%d", frame->data[1], frame->linesize[1], frame->width, frame->height);
            LOG_D("get frame v address:%p, size:%d, width:%d, height:%d", frame->data[2], frame->linesize[2], frame->width, frame->height);
            image_processor_(frame->data[0], frame->data[1], frame->data[2]);
        }
    }  
};