idf_component_register(SRCS 
            "src/GuitarHeroAdapter.cpp"
            "src/InputManager.cpp"
            "src/ConfigManager.cpp"
            INCLUDE_DIRS "include"
            REQUIRES driver esp_adc BleXInput)
