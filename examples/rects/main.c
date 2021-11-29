//
//  main.c
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

#include "sk_capi.h"

#define NUMBER_OF_SHAPES 1000

int winWidth = 512;
int winHeight = 512;

typedef struct {
    sk_rect_t rect;
    sk_ipoint_t moveBy;
    sk_color_t color;
} shape;

shape shapes[NUMBER_OF_SHAPES];

int randomNumber(int min, int max) {
    return rand() % (max + 1 - min) + min;
}

void moveShape(shape* shape) {
    int moveX = shape->moveBy.x;
    int moveY = shape->moveBy.y;

    sk_rect_t* rect = &(shape->rect);
    float left = rect->left;
    float top = rect->top;
    float w = rect->right - left;
    float h = rect->bottom - top;

    rect->left += moveX;
    rect->right += moveX;
    rect->top += moveY;
    rect->bottom += moveY;

    if ((left + moveX) < 0.0) {
        rect->left = (winWidth - w) + moveX;
        rect->right = winWidth + moveX;
    }
    if ((left + moveX) > (winWidth - w)) {
        rect->left = 0 + moveX;
        rect->right = w + moveX;
    }

    if ((top + moveY) < 0.0) {
        rect->top = (winHeight - h) + moveY;
        rect->bottom = winHeight + moveY;
    }

    if ((top + moveY) > (winHeight - h)) {
        rect->top = 0 + moveY;
        rect->bottom = h + moveY;
    }
}

sk_color_t colorForIndex(int index) {
    int r = index << 2;
    int g = index << 4;
    int b = (r + g) >> 2;
    return 0x80000000 | (r << 16) | (g << 8) | (b & 0xFF);
}

void draw(sk_canvas_t* canvas) {
    sk_canvas_draw_color(canvas, 0xFFFFFFFF, SK_BLEND_MODE_SRCOVER);

    sk_paint_t* paint = sk_paint_new();
    sk_paint_set_style(paint, SK_PAINT_STYLE_STROKE);
    sk_paint_set_stroke_width(paint, 2);

    for (int i = 0; i < NUMBER_OF_SHAPES; i++) {
        sk_paint_set_color(paint, shapes[i].color);
        moveShape(&shapes[i]);
        sk_canvas_draw_rect(canvas, &(shapes[i].rect), paint);
        //sk_canvas_draw_oval(canvas, &(shapes[i].rect), paint);
        //sk_canvas_draw_arc(canvas, &(shapes[i].rect), 0, i * 2, true, paint);
    }

    sk_paint_delete(paint);
}

static gr_direct_context_t* makeSkiaContext(void);
static sk_surface_t* newSurface(gr_direct_context_t* context, const int w, const int h);

int main(int argc, char** argv) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    GLFWwindow* window = glfwCreateWindow(winWidth, winHeight, "Canvas Paint", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    int frames = 0;
    double t, t0, fps;
    char titleBuffer[10];
    t = t0 = fps = 0.0;

    glfwMakeContextCurrent(window);
    gr_direct_context_t* context = makeSkiaContext();
    glfwSwapInterval(1);

    for (int i = 0; i < NUMBER_OF_SHAPES; i++) {
        float left = randomNumber(0, winWidth / 10);
        float top = randomNumber(0, winHeight / 10);
        float width = randomNumber(10, winWidth / 10);
        float height = randomNumber(10, winHeight / 10);

        shapes[i].moveBy.x = randomNumber(-5, 5);
        shapes[i].moveBy.y = randomNumber(-5, 5);
        shapes[i].rect.left = left;
        shapes[i].rect.top = top;
        shapes[i].rect.right = left + width;
        shapes[i].rect.bottom = top + height;
        shapes[i].color = colorForIndex(i);
    }

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

        t = glfwGetTime();
        if ((t - t0) > 0.5 || frames == 0) {
            fps = (double)frames / (t - t0);
            snprintf(titleBuffer, sizeof(titleBuffer), "FPS: %.2f", fps);
            glfwSetWindowTitle(window, titleBuffer);
            t0 = t;
            frames = 0;
        }
        frames++;
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

