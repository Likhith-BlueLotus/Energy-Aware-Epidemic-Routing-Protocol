set(source_files
    model/epidemic-packet-queue.cc
    model/epidemic-packet.cc
    model/epidemic-tag.cc
    model/energy-aware-epidemic-routing.cc
    helper/energy-aware-epidemic-helper.cc
)

set(header_files
    model/epidemic-packet-queue.h
    model/epidemic-packet.h
    model/epidemic-tag.h
    model/energy-aware-epidemic-routing.h
    helper/energy-aware-epidemic-helper.h
)

build_lib(
  LIBNAME epidemic
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK
    ${libinternet}
    ${libenergy}
  TEST_SOURCES
    test/epidemic-test-suite.cc
)
