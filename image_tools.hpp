#pragma once
static void convert_uyvy_i420(unsigned const char *uyvy,
                        int width,
                        int height,
                        unsigned char *i420) {
    int pixels_in_a_row = width << 1;
    int u_index = 0;
    int y_size = width * height;
    int uv_size = y_size / 4;
    unsigned char* i420_y = i420;
    unsigned char* i420_u = i420_y + y_size;
    unsigned char* i420_v = i420_u + uv_size;

    for (int i = 0;i < height;++i) {
        for (int j = 0;j < width;j += 2) {
            u_index = i * pixels_in_a_row + (j << 1);
            *i420_y++ = uyvy[u_index + 1];
            *i420_y++ = uyvy[u_index + 3];
            if (0 == (i & 0x01)) {      // odd lines
                *i420_u++ = uyvy[u_index];
                *i420_v++ = uyvy[u_index + 2];
            }
        }
    }     
}
static void convert_i420_uyvy(const unsigned char *i420,
                            int width,
                            int height,
                            unsigned char *uyvy) {
    int y_size = width * height;
    int uv_size = y_size / 4;
    const unsigned char* i420_y = i420;
    const unsigned char* i420_u = i420_y + y_size;
    const unsigned char* i420_v = i420_u + uv_size;
    int y_stride = width;
    int uv_stride = (width >> 1);
    int y_index = 0;
    int u_index = 0;
    int v_index = 0;
    for (int i = 0;i < height;++i) {
        for (int j = 0;j < width;j += 2) {
            v_index = u_index = (i >> 1) * uv_stride + (j >> 1);
            y_index = i * y_stride + j;
            uyvy[0] = i420_u[u_index];      // u
            uyvy[1] = i420_y[y_index];      // y
            uyvy[2] = i420_v[v_index];      // v
            uyvy[3] = i420_y[y_index + 1];  // y
            uyvy += 4;
        }
    }      
}
