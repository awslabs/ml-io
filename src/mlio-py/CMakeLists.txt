add_subdirectory(mlio/core)

if(MLIO_INCLUDE_CONTRIB)
endif()

if(MLIO_INCLUDE_ARROW_INTEGRATION)
    add_subdirectory(mlio/integ/arrow)
endif()
