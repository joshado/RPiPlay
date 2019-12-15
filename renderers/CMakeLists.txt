cmake_minimum_required(VERSION 3.4.1)

add_subdirectory( h264-bitstream )
add_library( renderers
        STATIC
        audio_renderer_dummy.c video_renderer_dummy.c)
target_link_libraries ( renderers h264-bitstream )
