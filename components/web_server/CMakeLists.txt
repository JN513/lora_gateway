idf_component_register(SRCS "api.cpp" "templates.cpp" "system.cpp" "web_server.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_https_server storage_manager time_manager communication mbedtls json
                    EMBED_TXTFILES ${project_dir}/server_certs/cacert.pem  ${project_dir}/server_certs/prvtkey.pem ${project_dir}/components/web_server/favicon.ico)
