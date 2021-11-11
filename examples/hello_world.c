
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

gr_direct_context_t* makeSkiaContext() {
    const gr_glinterface_t* interface = gr_glinterface_create_native_interface();
    gr_direct_context_t* context = gr_direct_context_make_gl(interface);
    assert(context != NULL);
    return context;
}

sk_surface_t* skiaSurface(gr_direct_context_t* context, const int w, const int h) {
    static gr_gl_framebufferinfo_t fbInfo = {
        .fFBOID = 0,
        .fFormat = GL_RGBA8};

    gr_backendrendertarget_t* target = gr_backendrendertarget_new_gl(w, h, 0, 0, &fbInfo);
    sk_color_type_t colorType = SK_COLOR_TYPE_RGBA_8888;

    sk_surface_t* surface = sk_surface_new_backend_render_target(context, target,
                                                                 GR_SURFACE_ORIGIN_BOTTOM_LEFT,
                                                                 colorType, NULL, NULL);
    gr_backendrendertarget_delete(target);
    assert(surface != NULL);
    return surface;
}

void draw(sk_canvas_t* canvas, const int canvasWidth, const int canvasHeight) {
    sk_paint_t* paint = sk_paint_new();

    sk_paint_set_color(paint, 0xFF808080);
    sk_canvas_draw_paint(canvas, paint);

    sk_paint_set_color(paint, 0x8FAA0080);
    sk_rect_t rect = {.left = 0, .top = 0, .right = 200, .bottom = 200};
    sk_canvas_draw_rect(canvas, &rect, paint);

    sk_paint_set_color(paint, 0xFFFF0000);
    sk_paint_set_stroke_width(paint, 1);
    sk_canvas_draw_line(canvas, 0, 0, canvasWidth, canvasHeight, paint);
    sk_canvas_draw_line(canvas, 0, canvasHeight * 0.9, canvasWidth, canvasHeight * 0.9, paint);
    sk_canvas_draw_line(canvas, 0, 200, canvasWidth, 200, paint);
    sk_canvas_draw_line(canvas, 200, 0, 200, canvasHeight, paint);

    sk_paint_delete(paint);
}

int main(int argc, char** argv) {
    int width = 500;
    int height = 500;

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }
    /*
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        //(uncomment to enable correct color spaces) glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
        glfwWindowHint(GLFW_STENCIL_BITS, 0);
        // glfwWindowHint(GLFW_ALPHA_BITS, 0);
        glfwWindowHint(GLFW_DEPTH_BITS, 0);
        // glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    */

    GLFWwindow* window = glfwCreateWindow(width, height, "Hello Skia World", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    gr_direct_context_t* context = makeSkiaContext();

    glfwSwapInterval(1);
    // glfwSetKeyCallback(window, key_callback);

    while (!glfwWindowShouldClose(window)) {
        float contentScaleX, contentScaleY;
        double canvasWidth, canvasHeight;
        glfwGetWindowSize(window, &width, &height);
        glfwGetWindowContentScale(window, &contentScaleX, &contentScaleY);
        canvasWidth = width * contentScaleX;
        canvasHeight = height * contentScaleY;

        // Surface is cheap(ish?) to create src: https://groups.google.com/g/skia-discuss/c/3c10MvyaSug
        sk_surface_t* surface = skiaSurface(context, canvasWidth, canvasHeight);
        sk_canvas_t* canvas = sk_surface_get_canvas(surface);

        draw(canvas, canvasWidth, canvasHeight);

        sk_canvas_flush(canvas);

        glfwSwapBuffers(window);
        sk_surface_unref(surface);

        glfwPollEvents();
    }

    if (context) {
        gr_direct_context_abandon_context(context);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
