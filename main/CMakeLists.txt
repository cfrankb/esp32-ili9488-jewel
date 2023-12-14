set(SOURCES 
    "disp_spi.cpp"
    "ili9488.cpp"
    "main.cpp"
    "esphelpers.cpp" 
    "tileset.cpp"
    "grid.cpp"
    "shape.cpp"
    "font.cpp"
    "joystick.cpp"
)
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    INCLUDE_DIRS "./include"
)
