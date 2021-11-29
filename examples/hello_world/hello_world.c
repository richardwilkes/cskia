//
//  hello_world.c
//
//  Created by Daniel Owsiański on 20/11/2021.
//

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
#include <string.h>  // strlen

#include "sk_capi.h"

int winWidth = 512;
int winHeight = 512;

void draw(sk_canvas_t* canvas) {
    sk_canvas_draw_color(canvas, 0xFFFFFFFF, SK_BLEND_MODE_SRCOVER);

    sk_paint_t* paint = sk_paint_new();
    sk_paint_set_style(paint, SK_PAINT_STYLE_STROKE);
    sk_paint_set_stroke_width(paint, 2);

    sk_font_t* font = sk_font_new_with_values(NULL, 60, 1.0, 0.0);
    const char* s = "Hello Skia World!";
    sk_text_blob_t* text = sk_textblob_make_from_text(s, strlen(s), font, SK_TEXT_ENCODING_UTF8);

    sk_paint_set_color(paint, 0xFFFF00FF);
    sk_canvas_draw_text_blob(canvas, text, 25, 100, paint);

    sk_paint_set_style(paint, SK_PAINT_STYLE_FILL);
    sk_paint_set_color(paint, 0xFF0000FF);
    sk_canvas_draw_text_blob(canvas, text, 25, 100 + 100, paint);

    sk_paint_set_style(paint, SK_PAINT_STYLE_STROKE_AND_FILL);
    sk_paint_set_color(paint, 0xFF00FFFF);
    sk_canvas_draw_text_blob(canvas, text, 25, 100 + 100 + 100, paint);


    sk_paint_set_style(paint, SK_PAINT_STYLE_STROKE_AND_FILL);
    sk_paint_set_color(paint, 0xFF00FFFF);
    sk_canvas_draw_text_blob(canvas, text, 25, 100 + 100 + 100 + 100, paint);
    sk_paint_set_style(paint, SK_PAINT_STYLE_FILL);
    sk_paint_set_color(paint, 0x40FF00FF);
    sk_canvas_draw_text_blob(canvas, text, 25, 100 + 100 + 100 + 100, paint);

    sk_font_delete(font);
    sk_paint_delete(paint);
}

static gr_direct_context_t* makeSkiaContext(void);
static sk_surface_t* newSurface(gr_direct_context_t* context, const int w, const int h);

int main(int argc, char** argv) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    GLFWwindow* window = glfwCreateWindow(winWidth, winHeight, "Hello World!", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }


    glfwMakeContextCurrent(window);
    gr_direct_context_t* context = makeSkiaContext();
    glfwSwapInterval(1);


    while (!glfwWindowShouldClose(window)) {
        //glfwWaitEvents();

        float contentScaleX, contentScaleY;
        double canvasWidth, canvasHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        glfwGetWindowContentScale(window, &contentScaleX, &contentScaleY);

        canvasWidth = winWidth * contentScaleX;
        canvasHeight = winHeight * contentScaleY;

        // Surface is cheap(ish?) to create src: https://groups.google.com/g/skia-discuss/c/3c10MvyaSug
        sk_surface_t* surface = newSurface(context, canvasWidth, canvasHeight);

        sk_canvas_t* canvas = sk_surface_get_canvas(surface);
        sk_canvas_scale(canvas, contentScaleX, contentScaleY);

        draw(canvas);

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

static gr_direct_context_t* makeSkiaContext() {
    const gr_glinterface_t* interface = gr_glinterface_create_native_interface();
    gr_direct_context_t* context = gr_direct_context_make_gl(interface);
    assert(context != NULL);
    return context;
}

static sk_surface_t* newSurface(gr_direct_context_t* context, const int w, const int h) {
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

