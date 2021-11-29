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
#include <string.h>  // strlen

#include <unistd.h>  // getcwd

#ifdef __APPLE__
#include <limits.h>  // PATH_MAX
#else
#include <linux/limits.h>
#endif

#include "sk_capi.h"

const sk_image_t* image = NULL;

// https://fiddle.skia.org/c/@skcanvas_paint
void draw(sk_canvas_t* canvas) {
    // canvas->drawColor(SK_ColorWHITE);
    sk_canvas_draw_color(canvas, 0xFFFFFFFF, SK_BLEND_MODE_SRCOVER);

    // SkPaint paint;
    // paint.setStyle(SkPaint::kStroke_Style);
    // paint.setStrokeWidth(4);
    // paint.setColor(SK_ColorRED);
    sk_paint_t* paint = sk_paint_new();
    sk_paint_set_style(paint, SK_PAINT_STYLE_STROKE);
    sk_paint_set_stroke_width(paint, 4);
    sk_paint_set_color(paint, 0xFFFF0000);

    // SkRect rect = SkRect::MakeXYWH(50, 50, 40, 60);
    // canvas->drawRect(rect, paint);
    sk_rect_t rect = {.left = 50, .top = 50, .right = 50 + 40, .bottom = 50 + 60};
    sk_canvas_draw_rect(canvas, &rect, paint);

    /*
     TODO:
     //    SkRRect oval;
     //    oval.setOval(rect);
     //    oval.offset(40, 60);
     //    paint.setColor(SK_ColorBLUE);
     //    canvas->drawRRect(oval, paint);
     */

    // paint.setColor(SK_ColorCYAN);
    // canvas->drawCircle(180, 50, 25, paint);
    sk_paint_set_color(paint, 0xFF00FFFF);
    sk_canvas_draw_circle(canvas, 180, 50, 20, paint);

    // rect.offset(80, 0);
    // paint.setColor(SK_ColorYELLOW);
    // canvas->drawRoundRect(rect, 10, 10, paint);
    sk_rect_t offetRect = {.left = rect.left + 80, .right = rect.right + 80, .top = rect.top + 0, .bottom = rect.bottom + 0};
    sk_paint_set_color(paint, 0xFFFFFF00);
    sk_canvas_draw_round_rect(canvas, &offetRect, 10, 10, paint);

    // SkPath path;
    // path.cubicTo(768, 0, -512, 256, 256, 256);
    // paint.setColor(SK_ColorGREEN);
    // canvas->drawPath(path, paint);
    sk_path_t* path = sk_path_new();
    sk_path_cubic_to(path, 768, 0, -512, 256, 256, 256);
    sk_paint_set_color(paint, 0xFF00FF00);
    sk_canvas_draw_path(canvas, path, paint);
    sk_path_delete(path);

    // canvas->drawImage(image, 128, 128, SkSamplingOptions(), &paint);
    float imageW, imageH;
    imageW = sk_image_get_width(image);
    imageH = sk_image_get_height(image);

    sk_rect_t srcRect = {.left = 0, .top = 0, .right = imageW, .bottom = imageH};
    sk_rect_t dstRect = {.left = 128, .top = 128, .right = 128 + imageW, .bottom = 128 + imageH};
    sk_sampling_options_t samplingOpts = {
        .useCubic = true,
        .cubic = {.B = 1 / 3.0, .C = 1 / 3.0},  // Mitchell
        .filter = SK_FILTER_MODE_NEAREST,
        .mipmap = SK_MIPMAP_MODE_NONE};
    sk_canvas_draw_image_rect(canvas, image, &srcRect, &dstRect, &samplingOpts, paint, SRC_RECT_CONSTRAINT_FAST);

    // SkRect rect2 = SkRect::MakeXYWH(0, 0, 40, 60);
    // canvas->drawImageRect(image, rect2, SkSamplingOptions(), &paint);
    sk_rect_t squeezedDstRect = {.left = 0, .top = 0, .right = 40, .bottom = 60};
    sk_canvas_draw_image_rect(canvas, image, &srcRect, &squeezedDstRect, &samplingOpts, paint, SRC_RECT_CONSTRAINT_FAST);

    // SkPaint paint2;
    // auto text = SkTextBlob::MakeFromString("Hello, Skia!", SkFont(nullptr, 18));
    // canvas->drawTextBlob(text.get(), 50, 25, paint2);
    sk_paint_t* paint2 = sk_paint_new();
    sk_font_t* font = sk_font_new_with_values(NULL, 18, 1.0, 0.0);
    const char* s = "Hello Skia!";
    sk_text_blob_t* text = sk_textblob_make_from_text(s, strlen(s), font, SK_TEXT_ENCODING_UTF8);
    sk_canvas_draw_text_blob(canvas, text, 50, 25, paint2);
    sk_font_delete(font);
    sk_paint_delete(paint2);

    sk_paint_delete(paint);
}

static gr_direct_context_t* makeSkiaContext(void);
static sk_surface_t* newSurface(gr_direct_context_t* context, const int w, const int h);
static const sk_image_t* loadImage(const char* imageFile);

int main(int argc, char** argv) {
    int width = 512;
    int height = 512;

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    image = loadImage("color_wheel.png");

    GLFWwindow* window = glfwCreateWindow(width, height, "Canvas Paint", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gr_direct_context_t* context = makeSkiaContext();
    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(window)) {
        float contentScaleX, contentScaleY;
        double canvasWidth, canvasHeight;
        glfwGetWindowSize(window, &width, &height);
        glfwGetWindowContentScale(window, &contentScaleX, &contentScaleY);

        canvasWidth = width * contentScaleX;
        canvasHeight = height * contentScaleY;

        // Surface is cheap(ish?) to create src: https://groups.google.com/g/skia-discuss/c/3c10MvyaSug
        sk_surface_t* surface = newSurface(context, canvasWidth, canvasHeight);
        sk_canvas_t* canvas = sk_surface_get_canvas(surface);

        sk_canvas_scale(canvas, contentScaleX * 2, contentScaleY * 2);
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

static const sk_image_t* loadImage(const char* imageFile) {
    char imagePath[PATH_MAX];
    int imageFileLen = (int)strlen(imageFile);
    // Leave space in imagePath for '/' + imageFile + '\0'
    int cwdBufferSize = sizeof(imagePath) - (1 + imageFileLen + 1);
    if (cwdBufferSize > 0 && getcwd(imagePath, cwdBufferSize) == NULL) {
        fprintf(stderr, "Failed to get the current working dir\n");
        exit(EXIT_FAILURE);
    }
    strncat(imagePath, "/", sizeof(imagePath) - 2);
    strncat(imagePath, imageFile, sizeof(imagePath) - imageFileLen - 1);

    FILE* file = fopen(imageFile, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", imagePath);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", imagePath);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", imagePath);
        exit(74);
    }

    sk_data_t* data = sk_data_new_with_copy(buffer, bytesRead);
    if (data == NULL) {
        fprintf(stderr, "Could not create data from file \"%s\".\n", imagePath);
        exit(74);
    }

    sk_image_t* image = sk_image_new_from_encoded(data);
    if (image == NULL) {
        fprintf(stderr, "Could not create image from file \"%s\".\n", imagePath);
        exit(74);
    }

    sk_data_unref(data);
    free((void*)buffer);
    fclose(file);

    return image;
}
