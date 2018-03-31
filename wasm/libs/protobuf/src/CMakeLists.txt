set(protobuf_src google/protobuf)

include_directories(.)

set(libprotobuf_lite_files
   ${protobuf_src}/arena.cc
   ${protobuf_src}/arenastring.cc
   ${protobuf_src}/extension_set.cc
   ${protobuf_src}/generated_message_util.cc
   ${protobuf_src}/io/coded_stream.cc
   ${protobuf_src}/io/zero_copy_stream.cc
   ${protobuf_src}/io/zero_copy_stream_impl_lite.cc
   ${protobuf_src}/message_lite.cc
   ${protobuf_src}/repeated_field.cc
   ${protobuf_src}/stubs/bytestream.cc
   ${protobuf_src}/stubs/common.cc
   ${protobuf_src}/stubs/int128.cc
   ${protobuf_src}/stubs/once.cc
   ${protobuf_src}/stubs/status.cc
   ${protobuf_src}/stubs/statusor.cc
   ${protobuf_src}/stubs/stringpiece.cc
   ${protobuf_src}/stubs/stringprintf.cc
   ${protobuf_src}/stubs/structurally_valid.cc
   ${protobuf_src}/stubs/strutil.cc
   ${protobuf_src}/stubs/time.cc
   ${protobuf_src}/wire_format_lite.cc
)

add_library(protobuf-lite STATIC ${libprotobuf_lite_files})
target_link_libraries(protobuf-lite ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(protobuf-lite PUBLIC ${CMAKE_SOURCE_DIR}/libs/protobuf/src)
