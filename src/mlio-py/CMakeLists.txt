add_subdirectory(mlio/core)

if(MLIO_INCLUDE_CONTRIB)
    add_subdirectory(mlio/contrib/insights)
endif()

if(MLIO_INCLUDE_ARROW_INTEGRATION)
    add_subdirectory(mlio/integ/arrow)
endif()
