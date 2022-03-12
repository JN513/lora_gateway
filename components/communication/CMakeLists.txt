idf_component_register(SRCS "mesh.c" "communication.c"
                    INCLUDE_DIRS "include"
                    REQUIRES mqtt storage_manager)
