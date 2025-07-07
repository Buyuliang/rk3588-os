cmake_minimum_required(VERSION 3.10)
project(spl C ASM)

# 工具链配置
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_ASM_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_AR aarch64-linux-gnu-ar)
set(CMAKE_OBJCOPY aarch64-linux-gnu-objcopy)
set(CMAKE_OBJDUMP aarch64-linux-gnu-objdump)

# 编译选项
set(CMAKE_C_FLAGS "-DCONFIG_ARM64 -nostdinc -fno-builtin -mstrict-align -ffunction-sections -fdata-sections -fno-common -ffixed-r9 -ffixed-x18 -pipe -march=armv8-a+nosimd -g -fno-stack-protector")
set(CMAKE_ASM_FLAGS "-D__ASSEMBLY__ -DCONFIG_ARM64 -nostdinc -march=armv8-a+nosimd -g")

# 头文件
include_directories(${CMAKE_SOURCE_DIR}/include)

# 开启详细输出
set(CMAKE_VERBOSE_MAKEFILE ON)

# 禁止默认链接库
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> -T${CMAKE_SOURCE_DIR}/spl.lds -nostdlib -g <OBJECTS> -o <TARGET>")

# 明确指定源文件
set(C_SOURCES
    src/main.c
    src/printf.c
    src/gicv3.c
)

set(ASM_SOURCES
    start.s
    gicv3_asm.S
)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(spl.elf ${C_SOURCES} ${ASM_SOURCES})

# 输出路径
set_target_properties(spl.elf PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist
)

# bin 文件
add_custom_command(
    TARGET spl.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary -S ${CMAKE_BINARY_DIR}/dist/spl.elf ${CMAKE_BINARY_DIR}/dist/spl.bin
    COMMENT "Generating spl.bin"
)

# 反汇编目标
add_custom_target(
    dis
    COMMAND ${CMAKE_OBJDUMP} -D -m aarch64 ${CMAKE_BINARY_DIR}/dist/spl.elf > ${CMAKE_BINARY_DIR}/dist/spl.dis
    DEPENDS spl.elf
    COMMENT "Generating spl.dis"
)
