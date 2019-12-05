# ------------------------------------------------------------
# Target: mlio-arrow
# ------------------------------------------------------------

add_python_extension(mlio-arrow arrow
    arrow_buffer.cxx
    arrow_file.cxx
    module.cxx
)

target_link_libraries(mlio-arrow
    PRIVATE
        arrow_shared mlio::mlio
)
