# ------------------------------------------------------------
# Target: mlio-arrow
# ------------------------------------------------------------

add_python_extension(mlio-arrow arrow
    arrow_buffer.cc
    arrow_file.cc
    module.cc
)

target_link_libraries(mlio-arrow
    PRIVATE
        arrow_shared mlio::mlio
)
