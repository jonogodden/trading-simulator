# CMake generated Testfile for 
# Source directory: C:/Users/jonat/Programming/trading simulator/tests
# Build directory: C:/Users/jonat/Programming/trading simulator/build/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(UnitTests "C:/Users/jonat/Programming/trading simulator/build/tests/Debug/unit_tests.exe")
  set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/jonat/Programming/trading simulator/tests/CMakeLists.txt;30;add_test;C:/Users/jonat/Programming/trading simulator/tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(UnitTests "C:/Users/jonat/Programming/trading simulator/build/tests/Release/unit_tests.exe")
  set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/jonat/Programming/trading simulator/tests/CMakeLists.txt;30;add_test;C:/Users/jonat/Programming/trading simulator/tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(UnitTests "C:/Users/jonat/Programming/trading simulator/build/tests/MinSizeRel/unit_tests.exe")
  set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/jonat/Programming/trading simulator/tests/CMakeLists.txt;30;add_test;C:/Users/jonat/Programming/trading simulator/tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(UnitTests "C:/Users/jonat/Programming/trading simulator/build/tests/RelWithDebInfo/unit_tests.exe")
  set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/jonat/Programming/trading simulator/tests/CMakeLists.txt;30;add_test;C:/Users/jonat/Programming/trading simulator/tests/CMakeLists.txt;0;")
else()
  add_test(UnitTests NOT_AVAILABLE)
endif()
