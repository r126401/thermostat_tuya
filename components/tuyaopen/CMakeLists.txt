# CMakeLists.txt para el componente TuyaOpen

# Nombre del componente
set(COMPONENT_NAME tuyaopen)
# MODULE_PATH
set(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)

message (STATUS "MODULE PATH ES: " ${MODULE_PATH} )

set(LWIP lwip-2.1.2)
set(MBEDTLS mbedtls-3.1.0)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error -Wno-unused-function -Wno-unused-const-variable -Wno-discarded-qualifiers -Wno-unused-but-set-variable -Wno-implicit-fallthrough -Wno-error=return-type")
include( ${MODULE_PATH}/liblwip/lwip-2.1.2/src/Filelists.cmake)


# Incluir todos los archivos fuente en el directorio src y sus subdirectorios
file(GLOB_RECURSE SRC_FILES
    #common
    "${MODULE_PATH}/common/backoffAlgorithm/source/*.c"
    "${MODULE_PATH}/utilities/*.c"
    #device_driver
    #"${MODULE_PATH}/device_driver/display/*.c"
    #"${MODULE_PATH}/device_driver/display/tft_spi/*.c"
    #"${MODULE_PATH}/device_driver/display/*.c"
    #"${MODULE_PATH}/device_driver/display/encoder/*.c"
    #"${MODULE_PATH}/device_driver/display/touch/*.c"
    #liblvgl
    #"${MODULE_PATH}/liblvgl/lvgl/src/*.c"
    #"${MODULE_PATH}/liblvgl/port/*.c"    
    #libcjson
    #"${MODULE_PATH}/libcjson/cJSON/*"
    #libhttp
    "${MODULE_PATH}/libhttp/coreHTTP/source/core_http_client.c"
    "${MODULE_PATH}/libhttp/src/http_download.c"
    "${MODULE_PATH}/libhttp/coreHTTP/source/dependency/3rdparty/http_parser/http_parser.c"
    "${MODULE_PATH}/libhttp/src/http_client_wrapper.c"
    #liblwip
    "${MODULE_PATH}/liblwip/${LWIP}/src/core/*.c"
    "${MODULE_PATH}/liblwip${LWIP}/src/api/*.c"
    "${MODULE_PATH}/liblwip/${LWIP}/src/netif/ethernet.c"
    "${MODULE_PATH}/liblwip/port/*.c"
    #libmqtt
    "${MODULE_PATH}/libmqtt/coreMQTT/source/*.c"
    "${MODULE_PATH}/libmqtt/src/*.c"
    #libtls
    "${MODULE_PATH}/libtls/src/*.c"
    "${MODULE_PATH}/libtls/${MBEDTLS}/library/*.c"
    #tal_bluetooth
    "${MODULE_PATH}/tal_bluetooth/src/*.c"
    "${MODULE_PATH}/tal_bluetooth/nimble/*.c"
    "${MODULE_PATH}/tal_bluetooth/nimble/host/*.c"
    #tal_cli
    "${MODULE_PATH}/tal_cli/src/*"
    #tal_driver
    "${MODULE_PATH}/tal_driver/src/*"
    #tal_kv
    "${MODULE_PATH}/tal_kv/littlefs/lfs_util.c" 
    "${MODULE_PATH}/tal_kv/littlefs/lfs.c"
    "${MODULE_PATH}/tal_kv/tal_kv.c" 
    "${MODULE_PATH}/tal_kv/kv_serialize.c"
    #tal_network
    "${MODULE_PATH}/tal_network/src/*.c"
    #tal_security
    "${MODULE_PATH}/tal_security/src/*.c"
    #tal_system
    "${MODULE_PATH}/tal_system/src/*.c"
    #tal_wifi
    "${MODULE_PATH}/tal_wifi/src/*.c"
    #tal_wired
    "${MODULE_PATH}/tal_wired/include"
    #tuya_cloud_service
    "${MODULE_PATH}/tuya_cloud_service/lan/*.c"
    "${MODULE_PATH}/tuya_cloud_service/netmgr/netmgr.c" 
    "${MODULE_PATH}/tuya_cloud_service/protocol/*.c" 
    "${MODULE_PATH}/tuya_cloud_service/schema/*.c" 
    "${MODULE_PATH}/tuya_cloud_service/cloud/*.c"
    "${MODULE_PATH}/tuya_cloud_service/tls/*.c"
    "${MODULE_PATH}/tuya_cloud_service/transport/*.c"
    "${MODULE_PATH}/tuya_cloud_service/netcfg/*.c"




        

)

if(CONFIG_ENABLE_WIFI STREQUAL "y")
    file(GLOB_RECURSE WIFI_SRCS 
        "${MODULE_PATH}/netmgr/netconn_wifi.c" 
        "${MODULE_PATH}/netcfg/*.c")
    list(APPEND SRC_FILES ${WIFI_SRCS})
    list(APPEND  LIB_PUBLIC_INC ${MODULE_PATH}/netcfg)    
endif()

if(CONFIG_ENABLE_WIRED STREQUAL "y")
    file(GLOB_RECURSE WIRED_SRCS 
        "${MODULE_PATH}/netmgr/netconn_wired.c")
    list(APPEND SRC_FILES ${WIRED_SRCS})
endif()

if(CONFIG_ENABLE_BLUETOOTH STREQUAL "y")
    file(GLOB_RECURSE BLE_SRCS  
        "${MODULE_PATH}/ble/*.c")
    list(APPEND SRC_FILES ${BLE_SRCS})
    list(APPEND  LIB_PUBLIC_INC ${MODULE_PATH}/ble) 
endif()

list(APPEND LIB_SRCS ${LWIP_SRCS})


# Recopila todos los directorios de inclusión

message(ERROR "SRC_FILES " vale ${SRC_FILES})



# LIB_PUBLIC_INC
set(LIB_PUBLIC_INC 
    #common
    "${MODULE_PATH}/common/include"
    "${MODULE_PATH}/common/utilities"
    "${MODULE_PATH}/common/backoffAlgorithm/source/include"
    
    #device_driver
    #"${MODULE_PATH}/device_driver/display/tft_spi"
    #liblvgl
    #"${MODULE_PATH}/liblvgl"
    "${MODULE_PATH}/liblvgl/lvgl"
    "${MODULE_PATH}/liblvgl/port"
    #libhttp
    "${MODULE_PATH}/libhttp/include"
    "${MODULE_PATH}/libhttp/coreHTTP" 
    "${MODULE_PATH}/libhttp/coreHTTP/source/include"
    "${MODULE_PATH}/libhttp/coreHTTP/source/dependency/3rdparty/http_parser"
    #liblwip
    "${MODULE_PATH}/liblwip/${LWIP}/src/include"
    "${MODULE_PATH}/liblwip/${LWIP}/src/include/lwip"
    "${MODULE_PATH}/liblwip/${LWIP}/src/include/compat"

    #libmqtt
    "${MODULE_PATH}/libmqtt/coreMQTT/source/include"
    "${MODULE_PATH}/libmqtt/include"
    #libtls
    "${MODULE_PATH}/libtls/include"
    "${MODULE_PATH}/libtls/port"
    "${MODULE_PATH}/libtls/${MBEDTLS}/include"
    #tal_bluetooth
    "${MODULE_PATH}/tal_bluetooth/include"
    "${MODULE_PATH}/tal_bluetooth/nimble/include"
    "${MODULE_PATH}/tal_bluetooth/nimble/host"

    #tal_cli
    "${MODULE_PATH}/tal_cli/include"
    #tal_driver
    "${MODULE_PATH}/tal_driver/include"
    #tal_kv
    "${MODULE_PATH}/tal_kv/include"
    "${MODULE_PATH}/tal_kv/littlefs"
    

    #tal_network
    "${MODULE_PATH}/tal_network/include"
    #tal_security
    "${MODULE_PATH}/tal_security/include"
    #tal_system
    "${MODULE_PATH}/tal_system/include"
    #tal_wifi
    "${MODULE_PATH}/tal_wifi/include"
    #tal_wired
    "${MODULE_PATH}/tal_wired/include"
    #tuya_cloud_service
    "${MODULE_PATH}/tuya_cloud_service/lan"
    "${MODULE_PATH}/tuya_cloud_service/netmgr" 
    "${MODULE_PATH}/tuya_cloud_service/protocol" 
    "${MODULE_PATH}/tuya_cloud_service/schema" 
    "${MODULE_PATH}/tuya_cloud_service/tls"
    "${MODULE_PATH}/tuya_cloud_service/transport"
    "${MODULE_PATH}/tuya_cloud_service/cloud"




)



message(STATUS "INCLUDE_DIRS " vale ${LIB_PUBLIC_INC})

    idf_component_register(
        PRIV_INCLUDE_DIRS "${MODULE_PATH}/tal_kv/port"
        SRCS ${SRC_FILES}         # Archivos fuente
        INCLUDE_DIRS ${LIB_PUBLIC_INC}
        REQUIRES tuyaos_adapter json
        
    )
    target_compile_definitions(${COMPONENT_LIB} PRIVATE LFS_CONFIG=lfs_config.h)
