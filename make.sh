#g++ -std=c++17 -g -o EncodeMP4Test encode_mp4.cpp -I ../install/usr/local/include/ -L ../install/usr/local/lib/  -lavcodec -lavutil -lswresample -pthread
#g++ -std=c++17 -g -o DecodeMP4Test decode_mp4.cpp -I ../install/usr/local/include/ -L ../install/usr/local/lib/  -lavcodec -lavutil -lswresample -pthread
qcc -lang-c++ -Vgcc_ntoaarch64le -g -o EncodeMP4Test encode_mp4.cpp -I ../install/usr/local/include/ -L ../install/usr/local/lib/  -lavcodec -lavutil -lswresample
qcc -lang-c++ -Vgcc_ntoaarch64le -g -o DecodeMP4Test decode_mp4.cpp -I ../install/usr/local/include/ -L ../install/usr/local/lib/  -lavcodec -lavutil -lswresample
