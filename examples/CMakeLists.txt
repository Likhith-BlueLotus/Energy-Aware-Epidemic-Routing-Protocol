set(base_examples
    energy-aware-epidemic-example
)

foreach(
  example
  ${base_examples}
)
  build_lib_example(
    NAME ${example}
    SOURCE_FILES ${example}.cc
    LIBRARIES_TO_LINK
      ${libepidemic}
      ${libwifi}
      ${libmobility}
      ${libapplications}
      ${libinternet}
      ${libnetwork}
      ${libcore}
      ${libenergy}
      ${libnetanim}
  )
endforeach()
