# ------------------------------------------------------------
# Target: mlio-py-arrow
# ------------------------------------------------------------

add_python_extension(mlio-py-arrow arrow
    arrow_buffer.cxx
    arrow_file.cxx
    module.cxx
)

target_link_libraries(mlio-py-arrow
    PRIVATE
        arrow_shared mlio::mlio
)
