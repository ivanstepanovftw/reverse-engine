# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
project(example)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Build executable                                                │
# └──────────────────────────────────────────────────────────────────┘
add_executable(trainer trainer.cc)
add_executable(csgo_linux64_fix_resources_download csgo_linux64/fix_resources_download/main.cc)



target_link_libraries(trainer
        libreverseengine
        pthread
        boost_iostreams
        )


target_link_libraries(csgo_linux64_fix_resources_download
        libreverseengine
        pthread
        curl
        boost_iostreams
        )
