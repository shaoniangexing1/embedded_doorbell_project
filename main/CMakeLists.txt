idf_component_register(SRCS "main.c"
                            "doorbell_codec.c" 
                            "doorbell_led.c" 
                            "doorbell_button.c"
                            "doorbell_wifi.c"
                            "doorbell_mqtt.c"
                            "doorbell_camera.c"
                    INCLUDE_DIRS "." 
                    EMBED_FILES "output.pcm")