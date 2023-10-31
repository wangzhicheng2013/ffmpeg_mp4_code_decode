#include <sys/time.h>
#include "ffmpeg_mp4_code_decode.hpp"
int main(int argc, const char**argv) {
    printf("usage:./EncodeMP4Test WIDTH HEIGHT I420_PIC_PATH FPS DURATION_SECONDS LOCAL_MP4_PATH\n");
    if (argc != 7) {
        return -1;
    }
    int pic_w = atoi(argv[1]);
    int pic_h = atoi(argv[2]);
    const char *pic_path = argv[3];
    FILE *file  = fopen(pic_path , "rb+");
    if (!file) {
        LOG_E("%s can not open!", pic_path);
        return -1;
    }
    int fps = atoi(argv[4]);
    int duration_seconds = atoi(argv[5]);
    const char* mp4_path = argv[6];
    printf("image width:%d,image heigh:%d,image path:%s,mp4 code fps:%d,code duration:%ds,mp4_path:%s\n",
                pic_w, pic_h, pic_path, fps, duration_seconds, mp4_path);
    size_t size = pic_w * pic_h * 3 / 2;
    unsigned char *img = (unsigned char *)malloc(size * sizeof(unsigned char));
    if (nullptr == img) {
        LOG_E("malloc failed!");
        fclose(file);
        return -1;
    }
    fread(img, size, 1, file);
    fclose(file);
    unsigned char* img_y = img;
    unsigned char* img_u = img + pic_w * pic_h;
    unsigned char* img_v = img_u + pic_w * pic_h / 4;
    file = nullptr;
    ffmpeg_mp4_code_decode code_mp4;
    code_mp4.set_scale(pic_w, pic_h);
    code_mp4.set_fps(fps);
    code_mp4.set_code_mp4_path(mp4_path);
    if (false == code_mp4.init_code()) {
        free(img);
        return -1;
    }
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    int loop = fps * duration_seconds;
    for (int i = 0;i < loop;i++) {
        *img_y = i % 256;
        code_mp4.code_i420_frame(img_y, img_u, img_v);
    }
    code_mp4.get_code_mp4();
    gettimeofday(&end_time, NULL);
    double time_used = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    printf("mp4 code time elapse:%lfs\n", time_used);
    free(img);

    return 0;
}