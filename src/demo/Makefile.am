noinst_PROGRAMS = demo
EXTRA_DIST = vertex.glsl fragment.glsl
demo_SOURCES = main.cpp camera.h camera.cpp window.h window.cpp graphics.h graphics.cpp shape_renderer.h 
demo_CPPFLAGS = -I$(top_srcdir)/src $(BOOST_CPPFLAGS) $(PTHREAD_CFLAGS) $(SDL2_CFLAGS) $(GL_CFLAGS) $(GLEW_CFLAGS) $(Bullet_CFLAGS)
demo_LDFLAGS = -L$(top_builddir)/src $(BOOST_LDFLAGS)
demo_LDADD = $(top_builddir)/src/libtdse.a $(BOOST_ASIO_LIB) $(PTHREAD_LIBS) $(WINSOCKETS_LIB) $(SDL2_LIBS) $(GL_LIBS) $(GLEW_LIBS) $(Bullet_LIBS)
