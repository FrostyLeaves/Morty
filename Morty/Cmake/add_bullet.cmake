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

    set(BULLET_INCLUDE
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/include/bullet"
    )

set( MORTY_CODE_DIRECTORIES ${MORTY_CODE_DIRECTORIES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/include/bullet
)

target_link_directories(Morty
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib
)


if(WIN32)
    set(PPRS "")
    set(PPOS_D "_Debug.lib")
    set(PPOS_R ".lib")
elseif(APPLE)
    set(PPRS "lib")
    set(PPOS_D ".a")
    set(PPOS_R ".a")
endif()

add_library(BULLET::LINEAR_MATH UNKNOWN IMPORTED)

set_target_properties(BULLET::LINEAR_MATH
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${BULLET_INCLUDE}"
                                IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/${PPRS}LinearMath${PPOS_D}"
                                IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/${PPRS}LinearMath${PPOS_R}"
)

add_library(BULLET::BULLET_DYNAMICS UNKNOWN IMPORTED)

set_target_properties(BULLET::BULLET_DYNAMICS
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${BULLET_INCLUDE}"
                                IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/${PPRS}BulletDynamics${PPOS_D}"
                                IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/${PPRS}BulletDynamics${PPOS_R}"
)


add_library(BULLET::BULLET_COLLISION UNKNOWN IMPORTED)

set_target_properties(BULLET::BULLET_COLLISION
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${BULLET_INCLUDE}"
                                IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/${PPRS}BulletCollision${PPOS_D}"
                                IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/Bullet/lib/${PPRS}BulletCollision${PPOS_R}"
)

add_library(BULLET::ALL INTERFACE IMPORTED)
    set_property(TARGET BULLET::ALL PROPERTY INTERFACE_LINK_LIBRARIES
                    BULLET::LINEAR_MATH
                    BULLET::BULLET_DYNAMICS
                    BULLET::BULLET_COLLISION
    )

set(BULLET_FOUND True)