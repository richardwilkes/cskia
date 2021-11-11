
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

// GLFW_INCLUDE_GLCOREARB makes the GLFW header include the modern OpenGL
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>  // EXIT_SUCCESS, EXIT_FAILURE

#include "sk_capi.h"

gr_direct_context_t* kContext = NULL;

sk_surface_t* initSkia(const int w, const int h) {
    const gr_glinterface_t* interface = gr_glinterface_create_native_interface();
    gr_direct_context_t* kContext = gr_direct_context_make_gl(interface);

    gr_gl_framebufferinfo_t fbInfo = {
        .fFBOID = 0,
        .fFormat = GL_RGBA8};

    gr_backendrendertarget_t* target = gr_backendrendertarget_new_gl(w, h, 0, 0, &fbInfo);
    sk_color_type_t colorType = SK_COLOR_TYPE_RGBA_8888;

    sk_surface_t* surface = sk_surface_new_backend_render_target(kContext, target, GR_SURFACE_ORIGIN_BOTTOM_LEFT, colorType, NULL, NULL);

    assert(surface != NULL);
    return surface;
}

int main(int argc, char** argv) {
    GLFWwindow* window;
    int width = 500;
    int height = 500;

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //(uncomment to enable correct color spaces) glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
    // glfwWindowHint(GLFW_ALPHA_BITS, 0);
    glfwWindowHint(GLFW_DEPTH_BITS, 0);

    window = glfwCreateWindow(width, height, "Hello Skia World", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    sk_surface_t* surface = initSkia(width, height);
    sk_canvas_t* canvas = sk_surface_get_canvas(surface);

    assert(canvas != NULL);
    glfwSwapInterval(1);
    // glfwSetKeyCallback(window, key_callback);

    while (!glfwWindowShouldClose(window)) {
        sk_paint_t* paint = sk_paint_new();

        sk_paint_set_color(paint, 0xFF80AA80);
        sk_canvas_draw_paint(canvas, paint);

        sk_paint_set_color(paint, 0xFFAA0080);
        sk_rect_t rect = {.left = 100, .top = 100, .right = 200, .bottom = 200};
        sk_canvas_draw_rect(canvas, &rect, paint);
        sk_canvas_flush(canvas);

        glfwSwapBuffers(window);
        sk_paint_delete(paint);

        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
