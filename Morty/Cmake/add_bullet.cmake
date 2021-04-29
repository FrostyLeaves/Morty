############################################################
# SDL
############################################################

#include(${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/cmake/bullet/BulletConfig.cmake)

#if(WIN32)
#    set(BULLET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/cmake/bullet)
#else()
#    set(BULLET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/cmake/bullet)
#endif()
#
#find_package(BULLET_DIR)

set( MORTY_CODE_DIRECTORIES ${MORTY_CODE_DIRECTORIES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/include/bullet
)

target_link_directories(Morty
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib
)

add_library(BULLET::LINEAR_MATH UNKNOWN IMPORTED)

set_target_properties(BULLET::LINEAR_MATH
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/LinearMath_Debug.lib"
                                IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/LinearMath.lib"
)


add_library(BULLET::BULLET_COMMON UNKNOWN IMPORTED)

set_target_properties(BULLET::BULLET_COMMON
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/Bullet3Common_Debug.lib"
                                IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/Bullet3Common.lib"
)


add_library(BULLET::BULLET_DYNAMICS UNKNOWN IMPORTED)

set_target_properties(BULLET::BULLET_DYNAMICS
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/Bullet3Dynamics_Debug.lib"
                                IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/Bullet3Dynamics.lib"
)


add_library(BULLET::BULLET_COLLISION UNKNOWN IMPORTED)

set_target_properties(BULLET::BULLET_COLLISION
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/Bullet3Collision_Debug.lib"
                                IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/Bullet3Collision.lib"
)

add_library(BULLET::ALL INTERFACE IMPORTED)
    set_property(TARGET BULLET::ALL PROPERTY INTERFACE_LINK_LIBRARIES
                    BULLET::LINEAR_MATH
                    BULLET::BULLET_COMMON
                    BULLET::BULLET_DYNAMICS
                    BULLET::BULLET_COLLISION
    )

set(BULLET_FOUND True)