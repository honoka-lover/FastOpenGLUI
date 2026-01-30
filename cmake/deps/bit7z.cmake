include(FetchContent)

FetchContent_Declare(
        bit7z
        GIT_REPOSITORY https://github.com/rikyoz/bit7z.git
        GIT_TAG        v4.0.10  # 或者某个稳定 commit
)

FetchContent_MakeAvailable(bit7z)
