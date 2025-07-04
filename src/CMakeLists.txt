set(TINYCLI_SOURCES
    tinycli.c
    command.c
    plugin.c
    utils.c
    context.c
)

# Create the TinyCLI library
add_library(tinycli SHARED ${TINYCLI_SOURCES})
target_link_libraries(tinycli
    ${READLINE_LIBRARIES}
    ${CJSON_LIBRARIES}
    dl  # For dynamic loading of plugins
)

# Create the TinyCLI executable
add_executable(tinycli-bin main.c)
set_target_properties(tinycli-bin PROPERTIES OUTPUT_NAME tinycli)
target_link_libraries(tinycli-bin tinycli ${READLINE_LIBRARIES})
