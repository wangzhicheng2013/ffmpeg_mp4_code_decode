//1655973 
//7075806
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
    #include "libavcodec/avcodec.h"
    #include <libavcodec/avcodec.h>
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libavutil/imgutils.h"
    #include "libavutil/avassert.h"
    #include "libavutil/imgutils.h"
    #include "libavutil/intreadwrite.h"
    #include <libavutil/opt.h>
}
static const size_t INBUF_SIZE = (1920*1080*3/2);

static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,
                   FILE *outfile)
{
    int ret;

    /* send the frame to the encoder */
    if (frame)
        printf("Send frame: %zd\n", frame->pts);

    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }


        printf("Write packet %zd (size=%5d) stream_index = %d\n", pkt->pts, pkt->size, pkt->stream_index);
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }
}
static void decode(AVCodecContext *dec_ctx,
                   AVFrame *frame,
                   AVPacket *pkt,
                   const char *filename) 
{
    char buf[1024];
    int ret;

    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }

        printf("saving frame %lu\n", dec_ctx->frame_num);
        fflush(stdout);

        /* the picture is allocated by the decoder. no need to
           free it */
        snprintf(buf, sizeof(buf), "%s-%lu", filename, dec_ctx->frame_num);
        printf("get frame:%p, size:%d, width:%d, height:%d, buf:%p\n", frame->data[0], frame->linesize[0], frame->width, frame->height, buf);
    	FILE *f = fopen("./I420_1920x1080.I420", "wb");

        fwrite(frame->data[0], 1, INBUF_SIZE, f);
    	fclose(f);

    }
}

int main() {
	const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
	if (nullptr == codec) {
		printf("codec is null!\n");
		return -1;
	}
	AVCodecContext *c = avcodec_alloc_context3(codec);
	if (nullptr == c) {
		fprintf(stderr, "Could not allocate video codec context\n");
       		exit(1);
    	}
	AVPacket *pkt = av_packet_alloc();
	if (!pkt) {
		exit(1);
	}
	//c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	c->bit_rate = 400000;
    	/* resolution must be a multiple of two */
   	c->width = 1920;
	c->height = 1080;
	/* frames per second */
   	c->time_base = (AVRational){1, 25};
    	c->framerate = (AVRational){25, 1};
	c->gop_size = 10;
   	c->max_b_frames = 1;
    	c->pix_fmt = AV_PIX_FMT_YUV420P;
	printf("ID:%d\n", codec->id);
	int ret = avcodec_open2(c, codec, NULL);
    	if (ret < 0) {
		fprintf(stderr, "Could not open codec:\n");
        	exit(1);
    	}
	printf("LAILE\n");
	FILE *f = fopen("./I420_1920x1080.yuv", "rb");
	if (!f) {
		fprintf(stderr, "Could not open\n");
 	        exit(1);
   	}
	AVFrame* frame = av_frame_alloc();
   	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
        	exit(1);
    	}
	frame->format = c->pix_fmt;
    	frame->width  = c->width;
    	frame->height = c->height;

   	ret = av_frame_get_buffer(frame, 0);
    	if (ret < 0) {
		fprintf(stderr, "Could not allocate the video frame data\n");
       		exit(1);
   	}
	const int SIZE = 1920 * 1080 * 3 / 2;
	uint8_t buf[SIZE] = {0};
	fread(buf, SIZE, 1, f);

	fclose(f);

	    /* encode 1 second of video */
	f = fopen("./66.mp4", "wb");
	int i = 0;
    //for (; i < 250; i++) {
    for (; i < 25; i++) {
        fflush(stdout);
        ret = av_frame_make_writable(frame);
        if (ret < 0)
            exit(1);

	frame->data[0] = buf;

	frame->data[1] = buf + 1920 * 1080;

	frame->data[2] = buf + 1920 * 1080 + 1920 * 1080 / 4;

        frame->pts = i;

        /* encode the image */
        encode(c, frame, pkt, f);
    }

    /* flush the encoder */
    encode(c, NULL, pkt, f);

    /* Add sequence end code to have a real MPEG file.
       It makes only sense because this tiny examples writes packets
       directly. This is called "elementary stream" and only works for some
       codecs. To create a valid file, you usually need to write packets
       into a proper file format or protocol; see mux.c.
     */
    fclose(f);

    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    // decode
    codec = avcodec_find_decoder(AV_CODEC_ID_MPEG4);
    AVCodecParserContext *parser = av_parser_init(codec->id);
    if (!parser) {
        fprintf(stderr, "parser not found\n");
        exit(1);
    }
    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */
    //c->width = 1920;
    //c->height = 1080;
    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }
    f = fopen("./66.mp4", "rb");
    if (!f) {
        fprintf(stderr, "Could not open\n");
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
    uint8_t *data;
    size_t   data_size;
    int eof;
    pkt = av_packet_alloc();


    do {
        /* read raw data from the input file */
        data_size = fread(inbuf, 1, INBUF_SIZE, f);
        if (ferror(f))
            break;
        eof = !data_size;

        /* use the parser to split the data into frames */
        data = inbuf;
        while (data_size > 0 || eof) {
		printf("LAILE1\n");
            ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                                   data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
		printf("LAILE2\n");
            if (ret < 0) {
                fprintf(stderr, "Error while parsing\n");
                exit(1);
            }
            data      += ret;
            data_size -= ret;

            if (pkt->size)
                decode(c, frame, pkt, "");
            else if (eof)
                break;
        }
    } while (!eof);

    /* flush the decoder */
    decode(c, frame, NULL, "");

    fclose(f);

    av_parser_close(parser);
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);

	
    return 0;
}

