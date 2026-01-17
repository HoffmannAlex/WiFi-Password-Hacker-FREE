# Add test executable
add_executable(hack_wifi_ai_tests
    test_wifi_network.cpp
    # Add more test files here
)

# Link test executable with the main library and GTest
target_link_libraries(hack_wifi_ai_tests
    PRIVATE
    hack_wifi_ai
    GTest::GTest
    GTest::Main
)

# Add test
add_test(NAME hack_wifi_ai_tests
    COMMAND hack_wifi_ai_tests
)
