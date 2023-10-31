#include <sys/time.h>
#include <time.h>
#include "ffmpeg_mp4_code_decode.hpp"
int pic_w = 1920;
int pic_h = 1080;
void decode_process(const unsigned char *i420_y, const unsigned char *i420_u, const unsigned char *i420_v) {
    char path[128] = { 0 };
    static int count = 0;
    snprintf(path, sizeof(path), "I420_%dx%d_%ld_%d.I420", pic_w, pic_h, time(nullptr), ++count);
    FILE* file = fopen(path, "a+");
    if (!file) {
        LOG_E("%s open failed!", path);
        return;
    }
    int size = pic_w * pic_h;
    int uv_size = size / 4;
    fwrite(i420_y, size, 1, file);
    fwrite(i420_u, uv_size, 1, file);
    fwrite(i420_v, uv_size, 1, file);
    fclose(file);
}
int main(int argc, const char**argv) {
    printf("usage:./DecodeMP4Test WIDTH HEIGHT LOCAL_MP4_PATH\n");
    if (argc != 2) {
        return -1;
    }
    const char *mp4_path = argv[1];
    printf("mp4_path:%s\n", mp4_path);
    ffmpeg_mp4_code_decode decode_mp4;
    decode_mp4.set_code_mp4_path(mp4_path);
    decode_mp4.set_image_processor(decode_process);
    if (false == decode_mp4.init_decode()) {
        return -1;
    }
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    decode_mp4.get_decode_i420_frame();
    gettimeofday(&end_time, NULL);
    double time_used = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    printf("mp4 decode time elapse:%lfs\n", time_used);

    return 0;
}