# Frozen Qt 6/Linux initial ABI baseline. Additions are allowed; removals fail.
find_program(NM nm REQUIRED)
execute_process(COMMAND "${NM}" -D --defined-only --demangle "${LIBRARY}"
  RESULT_VARIABLE result OUTPUT_VARIABLE symbols ERROR_VARIABLE error)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Cannot inspect ${LIBRARY}: ${error}")
endif()
file(STRINGS "${BASELINE}" required_symbols)
foreach(symbol IN LISTS required_symbols)
  string(FIND "${symbols}" " T ${symbol}\n" position)
  if(position EQUAL -1)
    message(FATAL_ERROR "Public API removed or changed: ${symbol}")
  endif()
endforeach()
