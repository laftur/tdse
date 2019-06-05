noinst_PROGRAMS = demo
EXTRA_DIST = vertex.glsl fragment.glsl
demo_SOURCES = main.cpp camera.h camera.cpp window.h window.cpp graphics.h graphics.cpp shape_renderer.h 
demo_CPPFLAGS = -I$(top_srcdir)/src $(BOOST_CPPFLAGS) $(PTHREAD_CFLAGS) $(SDL2_CFLAGS) $(GLEW_CFLAGS) $(Bullet_CFLAGS)
demo_LDFLAGS = -L$(top_builddir)/src $(BOOST_LDFLAGS) $(SDL2_LIBS) $(GLEW_LIBS) $(Bullet_LIBS)
demo_LDADD = -ltdse $(BOOST_ASIO_LIB) $(PTHREAD_LIBS) $(WINSOCKETS_LIB)
