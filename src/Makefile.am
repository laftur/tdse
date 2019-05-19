bin_PROGRAMS = tdse
tdse_SOURCES = main.cpp glm.h glm.cpp camera.h camera.cpp window.h window.cpp graphics.h graphics.cpp shape_renderer.h physics.h physics.cpp ship.h ship.cpp controller.h controller.cpp biped.h biped.cpp projectile.h projectile.cpp shooter.h shooter.cpp turret.h turret.cpp
tdse_CPPFLAGS = -DDATA_DIR='"$(pkgdatadir)"' $(BOOST_CPPFLAGS) $(PTHREAD_CFLAGS) $(SDL2_CFLAGS) $(GLEW_CFLAGS) $(Bullet_CFLAGS)
tdse_LDFLAGS = $(BOOST_LDFLAGS) $(PTHREAD_CFLAGS) $(SDL2_LIBS) $(GLEW_LIBS) $(Bullet_LIBS)
tdse_LDADD = $(BOOST_ASIO_LIB) $(PTHREAD_LIBS) $(WINSOCKETS_LIB)
