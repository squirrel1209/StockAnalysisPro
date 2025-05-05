# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\StockAnalysisPro_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\StockAnalysisPro_autogen.dir\\ParseCache.txt"
  "StockAnalysisPro_autogen"
  )
endif()
