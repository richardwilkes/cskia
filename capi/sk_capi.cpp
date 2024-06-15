#include "include/sk_capi.h"

#include "include/codec/SkBmpDecoder.h"
#include "include/codec/SkGifDecoder.h"
#include "include/codec/SkIcoDecoder.h"
#include "include/codec/SkJpegDecoder.h"
#include "include/codec/SkPngDecoder.h"
#include "include/codec/SkWbmpDecoder.h"
#include "include/codec/SkWebpDecoder.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkDocument.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/docs/SkPDFDocument.h"
#include "include/effects/Sk1DPathEffect.h"
#include "include/effects/Sk2DPathEffect.h"
#include "include/effects/SkColorMatrixFilter.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkHighContrastFilter.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/effects/SkShaderMaskFilter.h"
#include "include/effects/SkTableMaskFilter.h"
#include "include/effects/SkTrimPathEffect.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/pathops/SkPathOps.h"
#include "include/utils/SkParsePath.h"
#include "src/pdf/SkPDFDocumentPriv.h"

#if defined(SK_BUILD_FOR_MAC)
#include "include/ports/SkFontMgr_mac_ct.h"
#endif

#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
#include "include/ports/SkFontMgr_fontconfig.h"
#endif

#if defined(SK_BUILD_FOR_WIN)
#include "include/ports/SkTypeface_win.h"
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ASSERT_ENUM_MSG(SK, C) "enum values have changed for " TOSTRING(#SK) "; update " TOSTRING(#C) "."
#define ASSERT_STRUCT_MSG(SK, C) "struct definition has changed for " TOSTRING(#SK) "; update " TOSTRING(#C) "."

// ===== Verify enums =====

// gr_surface_origin_t
static_assert((int)GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin    == (int)GR_SURFACE_ORIGIN_TOP_LEFT,    ASSERT_ENUM_MSG(GrSurfaceOrigin, gr_surface_origin_t));
static_assert((int)GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin == (int)GR_SURFACE_ORIGIN_BOTTOM_LEFT, ASSERT_ENUM_MSG(GrSurfaceOrigin, gr_surface_origin_t));

// sk_alpha_type_t
static_assert((int)SkAlphaType::kUnknown_SkAlphaType  == (int)SK_ALPHA_TYPE_UNKNOWN,  ASSERT_ENUM_MSG(SkAlphaType, sk_alpha_type_t));
static_assert((int)SkAlphaType::kOpaque_SkAlphaType   == (int)SK_ALPHA_TYPE_OPAQUE,   ASSERT_ENUM_MSG(SkAlphaType, sk_alpha_type_t));
static_assert((int)SkAlphaType::kPremul_SkAlphaType   == (int)SK_ALPHA_TYPE_PREMUL,   ASSERT_ENUM_MSG(SkAlphaType, sk_alpha_type_t));
static_assert((int)SkAlphaType::kUnpremul_SkAlphaType == (int)SK_ALPHA_TYPE_UNPREMUL, ASSERT_ENUM_MSG(SkAlphaType, sk_alpha_type_t));
static_assert((int)SkAlphaType::kLastEnum_SkAlphaType == (int)SK_ALPHA_TYPE_LAST,     ASSERT_ENUM_MSG(SkAlphaType, sk_alpha_type_t));

// sk_blend_mode_t
static_assert((int)SkBlendMode::kClear             == (int)SK_BLEND_MODE_CLEAR,          ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kSrc               == (int)SK_BLEND_MODE_SRC,            ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kDst               == (int)SK_BLEND_MODE_DST,            ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kSrcOver           == (int)SK_BLEND_MODE_SRCOVER,        ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kDstOver           == (int)SK_BLEND_MODE_DSTOVER,        ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kSrcIn             == (int)SK_BLEND_MODE_SRCIN,          ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kDstIn             == (int)SK_BLEND_MODE_DSTIN,          ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kSrcOut            == (int)SK_BLEND_MODE_SRCOUT,         ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kDstOut            == (int)SK_BLEND_MODE_DSTOUT,         ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kSrcATop           == (int)SK_BLEND_MODE_SRCATOP,        ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kDstATop           == (int)SK_BLEND_MODE_DSTATOP,        ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kXor               == (int)SK_BLEND_MODE_XOR,            ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kPlus              == (int)SK_BLEND_MODE_PLUS,           ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kModulate          == (int)SK_BLEND_MODE_MODULATE,       ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kScreen            == (int)SK_BLEND_MODE_SCREEN,         ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kOverlay           == (int)SK_BLEND_MODE_OVERLAY,        ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kDarken            == (int)SK_BLEND_MODE_DARKEN,         ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kLighten           == (int)SK_BLEND_MODE_LIGHTEN,        ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kColorDodge        == (int)SK_BLEND_MODE_COLORDODGE,     ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kColorBurn         == (int)SK_BLEND_MODE_COLORBURN,      ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kHardLight         == (int)SK_BLEND_MODE_HARDLIGHT,      ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kSoftLight         == (int)SK_BLEND_MODE_SOFTLIGHT,      ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kDifference        == (int)SK_BLEND_MODE_DIFFERENCE,     ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kExclusion         == (int)SK_BLEND_MODE_EXCLUSION,      ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kMultiply          == (int)SK_BLEND_MODE_MULTIPLY,       ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kHue               == (int)SK_BLEND_MODE_HUE,            ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kSaturation        == (int)SK_BLEND_MODE_SATURATION,     ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kColor             == (int)SK_BLEND_MODE_COLOR,          ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kLuminosity        == (int)SK_BLEND_MODE_LUMINOSITY,     ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kLastCoeffMode     == (int)SK_BLEND_MODE_LAST_COEFF,     ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kLastSeparableMode == (int)SK_BLEND_MODE_LAST_SEPARABLE, ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));
static_assert((int)SkBlendMode::kLastMode          == (int)SK_BLEND_MODE_LAST,           ASSERT_ENUM_MSG(SkBlendMode, sk_blend_mode_t));

// sk_blur_style_t
static_assert((int)SkBlurStyle::kNormal_SkBlurStyle   == (int)SK_BLUR_STYLE_NORMAL, ASSERT_ENUM_MSG(SkBlurStyle, sk_blur_style_t));
static_assert((int)SkBlurStyle::kSolid_SkBlurStyle    == (int)SK_BLUR_STYLE_SOLID,  ASSERT_ENUM_MSG(SkBlurStyle, sk_blur_style_t));
static_assert((int)SkBlurStyle::kOuter_SkBlurStyle    == (int)SK_BLUR_STYLE_OUTER,  ASSERT_ENUM_MSG(SkBlurStyle, sk_blur_style_t));
static_assert((int)SkBlurStyle::kInner_SkBlurStyle    == (int)SK_BLUR_STYLE_INNER,  ASSERT_ENUM_MSG(SkBlurStyle, sk_blur_style_t));
static_assert((int)SkBlurStyle::kLastEnum_SkBlurStyle == (int)SK_BLUR_STYLE_LAST,   ASSERT_ENUM_MSG(SkBlurStyle, sk_blur_style_t));

// sk_clip_op_t
static_assert((int)SkClipOp::kDifference    == (int)SK_CLIP_OP_DIFFERENCE, ASSERT_ENUM_MSG(SkClipOp, sk_clip_op_t));
static_assert((int)SkClipOp::kIntersect     == (int)SK_CLIP_OP_INTERSECT,  ASSERT_ENUM_MSG(SkClipOp, sk_clip_op_t));
static_assert((int)SkClipOp::kMax_EnumValue == (int)SK_CLIP_OP_LAST,       ASSERT_ENUM_MSG(SkClipOp, sk_clip_op_t));

// sk_color_channel_t
static_assert((int)SkColorChannel::kR        == (int)SK_COLOR_CHANNEL_RED,   ASSERT_ENUM_MSG(SkColorChannel, sk_color_channel_t));
static_assert((int)SkColorChannel::kG        == (int)SK_COLOR_CHANNEL_GREEN, ASSERT_ENUM_MSG(SkColorChannel, sk_color_channel_t));
static_assert((int)SkColorChannel::kB        == (int)SK_COLOR_CHANNEL_BLUE,  ASSERT_ENUM_MSG(SkColorChannel, sk_color_channel_t));
static_assert((int)SkColorChannel::kA        == (int)SK_COLOR_CHANNEL_ALPHA, ASSERT_ENUM_MSG(SkColorChannel, sk_color_channel_t));
static_assert((int)SkColorChannel::kLastEnum == (int)SK_COLOR_CHANNEL_LAST,  ASSERT_ENUM_MSG(SkColorChannel, sk_color_channel_t));

// sk_color_type_t
static_assert((int)SkColorType::kUnknown_SkColorType            == (int)SK_COLOR_TYPE_UNKNOWN,            ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kAlpha_8_SkColorType            == (int)SK_COLOR_TYPE_ALPHA_8,            ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kRGB_565_SkColorType            == (int)SK_COLOR_TYPE_RGB_565,            ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kARGB_4444_SkColorType          == (int)SK_COLOR_TYPE_ARGB_4444,          ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kRGBA_8888_SkColorType          == (int)SK_COLOR_TYPE_RGBA_8888,          ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kBGRA_8888_SkColorType          == (int)SK_COLOR_TYPE_BGRA_8888,          ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kRGB_888x_SkColorType           == (int)SK_COLOR_TYPE_RGB_888X,           ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kRGBA_1010102_SkColorType       == (int)SK_COLOR_TYPE_RGBA_1010102,       ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kBGRA_1010102_SkColorType       == (int)SK_COLOR_TYPE_BGRA_1010102,       ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kRGB_101010x_SkColorType        == (int)SK_COLOR_TYPE_RGB_101010X,        ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kBGR_101010x_SkColorType        == (int)SK_COLOR_TYPE_BGR_101010X,        ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kBGR_101010x_XR_SkColorType     == (int)SK_COLOR_TYPE_BGR_101010X_XR,     ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kGray_8_SkColorType             == (int)SK_COLOR_TYPE_GRAY_8,             ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kRGBA_F16Norm_SkColorType       == (int)SK_COLOR_TYPE_RGBA_F16_NORM,      ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kRGBA_F16_SkColorType           == (int)SK_COLOR_TYPE_RGBA_F16,           ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kRGBA_F32_SkColorType           == (int)SK_COLOR_TYPE_RGBA_F32,           ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kR8G8_unorm_SkColorType         == (int)SK_COLOR_TYPE_R8G8_UNORM,         ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kA16_unorm_SkColorType          == (int)SK_COLOR_TYPE_A16_UNORM,          ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kR16G16_unorm_SkColorType       == (int)SK_COLOR_TYPE_R16G16_UNORM,       ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kA16_float_SkColorType          == (int)SK_COLOR_TYPE_A16_FLOAT,          ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kR16G16_float_SkColorType       == (int)SK_COLOR_TYPE_R16G16_FLOAT,       ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kR16G16B16A16_unorm_SkColorType == (int)SK_COLOR_TYPE_R16G16B16A16_UNORM, ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kSRGBA_8888_SkColorType         == (int)SK_COLOR_TYPE_SRGBA_8888,         ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kR8_unorm_SkColorType           == (int)SK_COLOR_TYPE_R8_UNORM,           ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));
static_assert((int)SkColorType::kLastEnum_SkColorType           == (int)SK_COLOR_TYPE_LAST,               ASSERT_ENUM_MSG(SkColorType, sk_color_type_t));

// sk_filter_mode_t
static_assert((int)SkFilterMode::kNearest == (int)SK_FILTER_MODE_NEAREST, ASSERT_ENUM_MSG(SkFilterMode, sk_filter_mode_t));
static_assert((int)SkFilterMode::kLinear  == (int)SK_FILTER_MODE_LINEAR,  ASSERT_ENUM_MSG(SkFilterMode, sk_filter_mode_t));
static_assert((int)SkFilterMode::kLast    == (int)SK_FILTER_MODE_LAST,    ASSERT_ENUM_MSG(SkFilterMode, sk_filter_mode_t));

// sk_font_hinting_t
static_assert((int)SkFontHinting::kNone   == (int)SK_FONT_HINTING_NONE,   ASSERT_ENUM_MSG(SkFontHinting, sk_font_hinting_t));
static_assert((int)SkFontHinting::kSlight == (int)SK_FONT_HINTING_SLIGHT, ASSERT_ENUM_MSG(SkFontHinting, sk_font_hinting_t));
static_assert((int)SkFontHinting::kNormal == (int)SK_FONT_HINTING_NORMAL, ASSERT_ENUM_MSG(SkFontHinting, sk_font_hinting_t));
static_assert((int)SkFontHinting::kFull   == (int)SK_FONT_HINTING_FULL,   ASSERT_ENUM_MSG(SkFontHinting, sk_font_hinting_t));

// sk_font_metrics_flags_t
static_assert((int)SkFontMetrics::FontMetricsFlags::kUnderlineThicknessIsValid_Flag == (int)SK_FONT_METRICS_FLAG_UNDERLINE_THICKNESS_IS_VALID, ASSERT_ENUM_MSG(SkFontMetrics::FontMetricsFlags, sk_font_metrics_flags_t));
static_assert((int)SkFontMetrics::FontMetricsFlags::kUnderlinePositionIsValid_Flag  == (int)SK_FONT_METRICS_FLAG_UNDERLINE_POSITION_IS_VALID,  ASSERT_ENUM_MSG(SkFontMetrics::FontMetricsFlags, sk_font_metrics_flags_t));
static_assert((int)SkFontMetrics::FontMetricsFlags::kStrikeoutThicknessIsValid_Flag == (int)SK_FONT_METRICS_FLAG_STRIKEOUT_THICKNESS_IS_VALID, ASSERT_ENUM_MSG(SkFontMetrics::FontMetricsFlags, sk_font_metrics_flags_t));
static_assert((int)SkFontMetrics::FontMetricsFlags::kStrikeoutPositionIsValid_Flag  == (int)SK_FONT_METRICS_FLAG_STRIKEOUT_POSITION_IS_VALID,  ASSERT_ENUM_MSG(SkFontMetrics::FontMetricsFlags, sk_font_metrics_flags_t));
static_assert((int)SkFontMetrics::FontMetricsFlags::kBoundsInvalid_Flag             == (int)SK_FONT_METRICS_FLAG_BOUNDS_INVALID,               ASSERT_ENUM_MSG(SkFontMetrics::FontMetricsFlags, sk_font_metrics_flags_t));

// sk_font_style_slant_t
static_assert((int)SkFontStyle::Slant::kUpright_Slant == (int)SK_FONT_STYLE_SLANT_UPRIGHT, ASSERT_ENUM_MSG(SkFontStyle::Slant, sk_font_style_slant_t));
static_assert((int)SkFontStyle::Slant::kItalic_Slant  == (int)SK_FONT_STYLE_SLANT_ITALIC,  ASSERT_ENUM_MSG(SkFontStyle::Slant, sk_font_style_slant_t));
static_assert((int)SkFontStyle::Slant::kOblique_Slant == (int)SK_FONT_STYLE_SLANT_OBLIQUE, ASSERT_ENUM_MSG(SkFontStyle::Slant, sk_font_style_slant_t));

// sk_font_style_weight_t
static_assert((int)SkFontStyle::Weight::kInvisible_Weight  == (int)SK_FONT_STYLE_WEIGHT_INVISIBLE,   ASSERT_ENUM_MSG(SkFontStyle::Weight, sk_font_style_weight_t));
static_assert((int)SkFontStyle::Weight::kThin_Weight       == (int)SK_FONT_STYLE_WEIGHT_THIN,        ASSERT_ENUM_MSG(SkFontStyle::Weight, sk_font_style_weight_t));
static_assert((int)SkFontStyle::Weight::kExtraLight_Weight == (int)SK_FONT_STYLE_WEIGHT_EXTRA_LIGHT, ASSERT_ENUM_MSG(SkFontStyle::Weight, sk_font_style_weight_t));
static_assert((int)SkFontStyle::Weight::kLight_Weight      == (int)SK_FONT_STYLE_WEIGHT_LIGHT,       ASSERT_ENUM_MSG(SkFontStyle::Weight, sk_font_style_weight_t));
static_assert((int)SkFontStyle::Weight::kNormal_Weight     == (int)SK_FONT_STYLE_WEIGHT_NORMAL,      ASSERT_ENUM_MSG(SkFontStyle::Weight, sk_font_style_weight_t));
static_assert((int)SkFontStyle::Weight::kMedium_Weight     == (int)SK_FONT_STYLE_WEIGHT_MEDIUM,      ASSERT_ENUM_MSG(SkFontStyle::Weight, sk_font_style_weight_t));
static_assert((int)SkFontStyle::Weight::kSemiBold_Weight   == (int)SK_FONT_STYLE_WEIGHT_SEMI_BOLD,   ASSERT_ENUM_MSG(SkFontStyle::Weight, sk_font_style_weight_t));
static_assert((int)SkFontStyle::Weight::kBold_Weight       == (int)SK_FONT_STYLE_WEIGHT_BOLD,        ASSERT_ENUM_MSG(SkFontStyle::Weight, sk_font_style_weight_t));
static_assert((int)SkFontStyle::Weight::kExtraBold_Weight  == (int)SK_FONT_STYLE_WEIGHT_EXTRA_BOLD,  ASSERT_ENUM_MSG(SkFontStyle::Weight, sk_font_style_weight_t));
static_assert((int)SkFontStyle::Weight::kBlack_Weight      == (int)SK_FONT_STYLE_WEIGHT_BLACK,       ASSERT_ENUM_MSG(SkFontStyle::Weight, sk_font_style_weight_t));
static_assert((int)SkFontStyle::Weight::kExtraBlack_Weight == (int)SK_FONT_STYLE_WEIGHT_EXTRA_BLACK, ASSERT_ENUM_MSG(SkFontStyle::Weight, sk_font_style_weight_t));

// sk_font_style_width_t
static_assert((int)SkFontStyle::Width::kUltraCondensed_Width == (int)SK_FONT_STYLE_WIDTH_ULTRA_CONDENSED, ASSERT_ENUM_MSG(SkFontStyle::Width, sk_font_style_width_t));
static_assert((int)SkFontStyle::Width::kExtraCondensed_Width == (int)SK_FONT_STYLE_WIDTH_EXTRA_CONDENSED, ASSERT_ENUM_MSG(SkFontStyle::Width, sk_font_style_width_t));
static_assert((int)SkFontStyle::Width::kCondensed_Width      == (int)SK_FONT_STYLE_WIDTH_CONDENSED,       ASSERT_ENUM_MSG(SkFontStyle::Width, sk_font_style_width_t));
static_assert((int)SkFontStyle::Width::kSemiCondensed_Width  == (int)SK_FONT_STYLE_WIDTH_SEMI_CONDENSED,  ASSERT_ENUM_MSG(SkFontStyle::Width, sk_font_style_width_t));
static_assert((int)SkFontStyle::Width::kNormal_Width         == (int)SK_FONT_STYLE_WIDTH_NORMAL,          ASSERT_ENUM_MSG(SkFontStyle::Width, sk_font_style_width_t));
static_assert((int)SkFontStyle::Width::kSemiExpanded_Width   == (int)SK_FONT_STYLE_WIDTH_SEMI_EXPANDED,   ASSERT_ENUM_MSG(SkFontStyle::Width, sk_font_style_width_t));
static_assert((int)SkFontStyle::Width::kExpanded_Width       == (int)SK_FONT_STYLE_WIDTH_EXPANDED,        ASSERT_ENUM_MSG(SkFontStyle::Width, sk_font_style_width_t));
static_assert((int)SkFontStyle::Width::kExtraExpanded_Width  == (int)SK_FONT_STYLE_WIDTH_EXTRA_EXPANDED,  ASSERT_ENUM_MSG(SkFontStyle::Width, sk_font_style_width_t));
static_assert((int)SkFontStyle::Width::kUltraExpanded_Width  == (int)SK_FONT_STYLE_WIDTH_ULTRA_EXPANDED,  ASSERT_ENUM_MSG(SkFontStyle::Width, sk_font_style_width_t));

// sk_high_contrast_config_invert_style_t
static_assert((int)SkHighContrastConfig::InvertStyle::kNoInvert         == (int)SK_HIGH_CONTRAST_CONFIG_INVERT_STYLE_NO_INVERT,         ASSERT_ENUM_MSG(SkHighContrastConfig::InvertStyle, sk_high_contrast_config_invert_style_t));
static_assert((int)SkHighContrastConfig::InvertStyle::kInvertBrightness == (int)SK_HIGH_CONTRAST_CONFIG_INVERT_STYLE_INVERT_BRIGHTNESS, ASSERT_ENUM_MSG(SkHighContrastConfig::InvertStyle, sk_high_contrast_config_invert_style_t));
static_assert((int)SkHighContrastConfig::InvertStyle::kInvertLightness  == (int)SK_HIGH_CONTRAST_CONFIG_INVERT_STYLE_INVERT_LIGHTNESS,  ASSERT_ENUM_MSG(SkHighContrastConfig::InvertStyle, sk_high_contrast_config_invert_style_t));
static_assert((int)SkHighContrastConfig::InvertStyle::kLast             == (int)SK_HIGH_CONTRAST_CONFIG_INVERT_STYLE_LAST,              ASSERT_ENUM_MSG(SkHighContrastConfig::InvertStyle, sk_high_contrast_config_invert_style_t));

// sk_image_caching_hint_t
static_assert((int)SkImage::CachingHint::kAllow_CachingHint    == (int)SK_IMAGE_CACHING_HINT_ALLOW,    ASSERT_ENUM_MSG(SkImage::CachingHint, sk_image_caching_hint_t));
static_assert((int)SkImage::CachingHint::kDisallow_CachingHint == (int)SK_IMAGE_CACHING_HINT_DISALLOW, ASSERT_ENUM_MSG(SkImage::CachingHint, sk_image_caching_hint_t));

// sk_mipmap_mode_t
static_assert((int)SkMipmapMode::kNone    == (int)SK_MIPMAP_MODE_NONE,    ASSERT_ENUM_MSG(SkMipmapMode, sk_mipmap_mode_t));
static_assert((int)SkMipmapMode::kNearest == (int)SK_MIPMAP_MODE_NEAREST, ASSERT_ENUM_MSG(SkMipmapMode, sk_mipmap_mode_t));
static_assert((int)SkMipmapMode::kLinear  == (int)SK_MIPMAP_MODE_LINEAR,  ASSERT_ENUM_MSG(SkMipmapMode, sk_mipmap_mode_t));
static_assert((int)SkMipmapMode::kLast    == (int)SK_MIPMAP_MODE_LAST,    ASSERT_ENUM_MSG(SkMipmapMode, sk_mipmap_mode_t));

// sk_paint_style_t
static_assert((int)SkPaint::Style::kFill_Style          == (int)SK_PAINT_STYLE_FILL,            ASSERT_ENUM_MSG(SkPaint::Style, sk_paint_style_t));
static_assert((int)SkPaint::Style::kStroke_Style        == (int)SK_PAINT_STYLE_STROKE,          ASSERT_ENUM_MSG(SkPaint::Style, sk_paint_style_t));
static_assert((int)SkPaint::Style::kStrokeAndFill_Style == (int)SK_PAINT_STYLE_STROKE_AND_FILL, ASSERT_ENUM_MSG(SkPaint::Style, sk_paint_style_t));

// sk_path_add_mode_t
static_assert((int)SkPath::AddPathMode::kAppend_AddPathMode == (int)SK_PATH_ADD_MODE_APPEND, ASSERT_ENUM_MSG(SkPath::AddPathMode, sk_path_add_mode_t));
static_assert((int)SkPath::AddPathMode::kExtend_AddPathMode == (int)SK_PATH_ADD_MODE_EXTEND, ASSERT_ENUM_MSG(SkPath::AddPathMode, sk_path_add_mode_t));

// sk_path_arc_size_t
static_assert((int)SkPath::ArcSize::kSmall_ArcSize == (int)SK_PATH_ARC_SIZE_SMALL, ASSERT_ENUM_MSG(SkPath::ArcSize, sk_path_arc_size_t));
static_assert((int)SkPath::ArcSize::kLarge_ArcSize == (int)SK_PATH_ARC_SIZE_LARGE, ASSERT_ENUM_MSG(SkPath::ArcSize, sk_path_arc_size_t));

// sk_path_direction_t
static_assert((int)SkPathDirection::kCW  == (int)SK_PATH_DIRECTION_CW,  ASSERT_ENUM_MSG(SkPathDirection, sk_path_direction_t));
static_assert((int)SkPathDirection::kCCW == (int)SK_PATH_DIRECTION_CCW, ASSERT_ENUM_MSG(SkPathDirection, sk_path_direction_t));

// sk_path_effect_1d_style_t
static_assert((int)SkPath1DPathEffect::Style::kTranslate_Style == (int)SK_PATH_EFFECT_1D_STYLE_TRANSLATE, ASSERT_ENUM_MSG(SkPath1DPathEffect::Style, sk_path_effect_1d_style_t));
static_assert((int)SkPath1DPathEffect::Style::kRotate_Style    == (int)SK_PATH_EFFECT_1D_STYLE_ROTATE,    ASSERT_ENUM_MSG(SkPath1DPathEffect::Style, sk_path_effect_1d_style_t));
static_assert((int)SkPath1DPathEffect::Style::kMorph_Style     == (int)SK_PATH_EFFECT_1D_STYLE_MORPH,     ASSERT_ENUM_MSG(SkPath1DPathEffect::Style, sk_path_effect_1d_style_t));
static_assert((int)SkPath1DPathEffect::Style::kLastEnum_Style  == (int)SK_PATH_EFFECT_1D_STYLE_LAST,      ASSERT_ENUM_MSG(SkPath1DPathEffect::Style, sk_path_effect_1d_style_t));

// sk_path_effect_trim_mode_t
static_assert((int)SkTrimPathEffect::Mode::kNormal   == (int)SK_PATH_EFFECT_TRIM_MODE_NORMAL,   ASSERT_ENUM_MSG(SkTrimPathEffect::Mode, sk_path_effect_trim_mode_t));
static_assert((int)SkTrimPathEffect::Mode::kInverted == (int)SK_PATH_EFFECT_TRIM_MODE_INVERTED, ASSERT_ENUM_MSG(SkTrimPathEffect::Mode, sk_path_effect_trim_mode_t));

// sk_path_fill_type_t
static_assert((int)SkPathFillType::kWinding        == (int)SK_PATH_FILLTYPE_WINDING,         ASSERT_ENUM_MSG(SkPathFillType, sk_path_fill_type_t));
static_assert((int)SkPathFillType::kEvenOdd        == (int)SK_PATH_FILLTYPE_EVENODD,         ASSERT_ENUM_MSG(SkPathFillType, sk_path_fill_type_t));
static_assert((int)SkPathFillType::kInverseWinding == (int)SK_PATH_FILLTYPE_INVERSE_WINDING, ASSERT_ENUM_MSG(SkPathFillType, sk_path_fill_type_t));
static_assert((int)SkPathFillType::kInverseEvenOdd == (int)SK_PATH_FILLTYPE_INVERSE_EVENODD, ASSERT_ENUM_MSG(SkPathFillType, sk_path_fill_type_t));

// sk_path_op_t
static_assert((int)SkPathOp::kDifference_SkPathOp        == (int)SK_PATH_OP_DIFFERENCE,         ASSERT_ENUM_MSG(SkPathOp, sk_path_op_t));
static_assert((int)SkPathOp::kIntersect_SkPathOp         == (int)SK_PATH_OP_INTERSECT,          ASSERT_ENUM_MSG(SkPathOp, sk_path_op_t));
static_assert((int)SkPathOp::kUnion_SkPathOp             == (int)SK_PATH_OP_UNION,              ASSERT_ENUM_MSG(SkPathOp, sk_path_op_t));
static_assert((int)SkPathOp::kXOR_SkPathOp               == (int)SK_PATH_OP_XOR,                ASSERT_ENUM_MSG(SkPathOp, sk_path_op_t));
static_assert((int)SkPathOp::kReverseDifference_SkPathOp == (int)SK_PATH_OP_REVERSE_DIFFERENCE, ASSERT_ENUM_MSG(SkPathOp, sk_path_op_t));

// sk_pixel_geometry_t
static_assert((int)SkPixelGeometry::kUnknown_SkPixelGeometry == (int)SK_PIXEL_GEOMETRY_UNKNOWN, ASSERT_ENUM_MSG(SkPixelGeometry, sk_pixel_geometry_t));
static_assert((int)SkPixelGeometry::kRGB_H_SkPixelGeometry   == (int)SK_PIXEL_GEOMETRY_RGB_H,   ASSERT_ENUM_MSG(SkPixelGeometry, sk_pixel_geometry_t));
static_assert((int)SkPixelGeometry::kBGR_H_SkPixelGeometry   == (int)SK_PIXEL_GEOMETRY_BGR_H,   ASSERT_ENUM_MSG(SkPixelGeometry, sk_pixel_geometry_t));
static_assert((int)SkPixelGeometry::kRGB_V_SkPixelGeometry   == (int)SK_PIXEL_GEOMETRY_RGB_V,   ASSERT_ENUM_MSG(SkPixelGeometry, sk_pixel_geometry_t));
static_assert((int)SkPixelGeometry::kBGR_V_SkPixelGeometry   == (int)SK_PIXEL_GEOMETRY_BGR_V,   ASSERT_ENUM_MSG(SkPixelGeometry, sk_pixel_geometry_t));

// sk_point_mode_t
static_assert((int)SkCanvas::PointMode::kPoints_PointMode  == (int)SK_POINT_MODE_POINTS,  ASSERT_ENUM_MSG(SkCanvas::PointMode, sk_point_mode_t));
static_assert((int)SkCanvas::PointMode::kLines_PointMode   == (int)SK_POINT_MODE_LINES,   ASSERT_ENUM_MSG(SkCanvas::PointMode, sk_point_mode_t));
static_assert((int)SkCanvas::PointMode::kPolygon_PointMode == (int)SK_POINT_MODE_POLYGON, ASSERT_ENUM_MSG(SkCanvas::PointMode, sk_point_mode_t));

// sk_src_rect_constraint_t
static_assert((int)SkCanvas::SrcRectConstraint::kStrict_SrcRectConstraint == (int)SRC_RECT_CONSTRAINT_STRICT, ASSERT_ENUM_MSG(SkCanvas::SrcRectConstraint, sk_src_rect_constraint_t));
static_assert((int)SkCanvas::SrcRectConstraint::kFast_SrcRectConstraint   == (int)SRC_RECT_CONSTRAINT_FAST,   ASSERT_ENUM_MSG(SkCanvas::SrcRectConstraint, sk_src_rect_constraint_t));

// sk_stroke_cap_t
static_assert((int)SkPaint::Cap::kButt_Cap    == (int)SK_STROKE_CAP_BUTT,    ASSERT_ENUM_MSG(SkPaint::Cap, sk_stroke_cap_t));
static_assert((int)SkPaint::Cap::kRound_Cap   == (int)SK_STROKE_CAP_ROUND,   ASSERT_ENUM_MSG(SkPaint::Cap, sk_stroke_cap_t));
static_assert((int)SkPaint::Cap::kSquare_Cap  == (int)SK_STROKE_CAP_SQUARE,  ASSERT_ENUM_MSG(SkPaint::Cap, sk_stroke_cap_t));
static_assert((int)SkPaint::Cap::kSquare_Cap  == (int)SK_STROKE_CAP_LAST,    ASSERT_ENUM_MSG(SkPaint::Cap, sk_stroke_cap_t));
static_assert((int)SkPaint::Cap::kDefault_Cap == (int)SK_STROKE_CAP_DEFAULT, ASSERT_ENUM_MSG(SkPaint::Cap, sk_stroke_cap_t));

// sk_stroke_join_t
static_assert((int)SkPaint::Join::kMiter_Join   == (int)SK_STROKE_JOIN_MITER,   ASSERT_ENUM_MSG(SkPaint::Join, sk_stroke_join_t));
static_assert((int)SkPaint::Join::kRound_Join   == (int)SK_STROKE_JOIN_ROUND,   ASSERT_ENUM_MSG(SkPaint::Join, sk_stroke_join_t));
static_assert((int)SkPaint::Join::kBevel_Join   == (int)SK_STROKE_JOIN_BEVEL,   ASSERT_ENUM_MSG(SkPaint::Join, sk_stroke_join_t));
static_assert((int)SkPaint::Join::kLast_Join    == (int)SK_STROKE_JOIN_LAST,    ASSERT_ENUM_MSG(SkPaint::Join, sk_stroke_join_t));
static_assert((int)SkPaint::Join::kDefault_Join == (int)SK_STROKE_JOIN_DEFAULT, ASSERT_ENUM_MSG(SkPaint::Join, sk_stroke_join_t));

// sk_text_encoding_t
static_assert((int)SkTextEncoding::kUTF8    == (int)SK_TEXT_ENCODING_UTF8,     ASSERT_ENUM_MSG(SkTextEncoding, sk_text_encoding_t));
static_assert((int)SkTextEncoding::kUTF16   == (int)SK_TEXT_ENCODING_UTF16,    ASSERT_ENUM_MSG(SkTextEncoding, sk_text_encoding_t));
static_assert((int)SkTextEncoding::kUTF32   == (int)SK_TEXT_ENCODING_UTF32,    ASSERT_ENUM_MSG(SkTextEncoding, sk_text_encoding_t));
static_assert((int)SkTextEncoding::kGlyphID == (int)SK_TEXT_ENCODING_GLYPH_ID, ASSERT_ENUM_MSG(SkTextEncoding, sk_text_encoding_t));

// sk_tile_mode_t
static_assert((int)SkTileMode::kClamp        == (int)SK_TILE_MODE_CLAMP, ASSERT_ENUM_MSG(SkTileMode, sk_tile_mode_t));
static_assert((int)SkTileMode::kRepeat       == (int)SK_TILE_REPEAT,     ASSERT_ENUM_MSG(SkTileMode, sk_tile_mode_t));
static_assert((int)SkTileMode::kMirror       == (int)SK_TILE_MIRROR,     ASSERT_ENUM_MSG(SkTileMode, sk_tile_mode_t));
static_assert((int)SkTileMode::kDecal        == (int)SK_TILE_DECAL,      ASSERT_ENUM_MSG(SkTileMode, sk_tile_mode_t));
static_assert((int)SkTileMode::kLastTileMode == (int)SK_TILE_LAST,       ASSERT_ENUM_MSG(SkTileMode, sk_tile_mode_t));

// ===== Verify structs =====

// Note: sk_matrix_t and sk_image_info_t have been left out, since they require special handling
static_assert(sizeof(gr_gl_framebufferinfo_t) == sizeof(GrGLFramebufferInfo), ASSERT_STRUCT_MSG(GrGLFramebufferInfo, gr_gl_framebufferinfo_t));
static_assert(sizeof(sk_cubic_resampler_t) == sizeof(SkCubicResampler), ASSERT_STRUCT_MSG(SkCubicResampler, sk_cubic_resampler_t));
static_assert(sizeof(sk_font_metrics_t) == sizeof(SkFontMetrics), ASSERT_STRUCT_MSG(SkFontMetrics, sk_font_metrics_t));
static_assert(sizeof(sk_high_contrast_config_t) == sizeof(SkHighContrastConfig), ASSERT_STRUCT_MSG(SkHighContrastConfig, sk_high_contrast_config_t));
static_assert(sizeof(sk_ipoint_t) == sizeof(SkIPoint), ASSERT_STRUCT_MSG(SkIPoint, sk_ipoint_t));
static_assert(sizeof(sk_irect_t) == sizeof(SkIRect), ASSERT_STRUCT_MSG(SkIRect, sk_irect_t));
static_assert(sizeof(sk_isize_t) == sizeof(SkISize), ASSERT_STRUCT_MSG(SkISize, sk_isize_t));
static_assert(sizeof(sk_point3_t) == sizeof(SkPoint3), ASSERT_STRUCT_MSG(SkPoint3, sk_point3_t));
static_assert(sizeof(sk_point_t) == sizeof(SkPoint), ASSERT_STRUCT_MSG(SkPoint, sk_point_t));
static_assert(sizeof(sk_rect_t) == sizeof(SkRect), ASSERT_STRUCT_MSG(SkRect, sk_rect_t));
static_assert(sizeof(sk_sampling_options_t) == sizeof(SkSamplingOptions), ASSERT_STRUCT_MSG(SkSamplingOptions, sk_sampling_options_t));
static_assert(sizeof(sk_text_blob_builder_run_buffer_t) == sizeof(SkTextBlobBuilder::RunBuffer), ASSERT_STRUCT_MSG(SkTextBlobBuilder::RunBuffer, sk_text_blob_builder_run_buffer_t));

static inline SkMatrix AsMatrix(const sk_matrix_t* matrix) {
    return SkMatrix::MakeAll(
        matrix->scaleX, matrix->skewX,  matrix->transX,
        matrix->skewY,  matrix->scaleY, matrix->transY,
        matrix->persp0, matrix->persp1, matrix->persp2);
}

static inline sk_matrix_t ToMatrix(const SkMatrix* matrix) {
    sk_matrix_t m;
    m.scaleX = matrix->get(SkMatrix::kMScaleX);
    m.skewX  = matrix->get(SkMatrix::kMSkewX);
    m.transX = matrix->get(SkMatrix::kMTransX);
    m.skewY  = matrix->get(SkMatrix::kMSkewY);
    m.scaleY = matrix->get(SkMatrix::kMScaleY);
    m.transY = matrix->get(SkMatrix::kMTransY);
    m.persp0 = matrix->get(SkMatrix::kMPersp0);
    m.persp1 = matrix->get(SkMatrix::kMPersp1);
    m.persp2 = matrix->get(SkMatrix::kMPersp2);
    return m;
}

static inline SkImageInfo AsImageInfo(const sk_image_info_t* info) {
    return SkImageInfo::Make(
        info->width,
        info->height,
        (SkColorType)info->colorType,
        (SkAlphaType)info->alphaType,
        sk_ref_sp(reinterpret_cast<SkColorSpace*>(info->colorSpace)));
}

// ===== Functions from include/gpu/GrBackendSurface.h =====
gr_backendrendertarget_t* gr_backendrendertarget_new_gl(int width, int height, int samples, int stencils, const gr_gl_framebufferinfo_t* glInfo) {
    return reinterpret_cast<gr_backendrendertarget_t*>(new GrBackendRenderTarget(GrBackendRenderTargets::MakeGL(width, height,
        samples, stencils, *reinterpret_cast<const GrGLFramebufferInfo*>(glInfo))));
}

void gr_backendrendertarget_delete(gr_backendrendertarget_t* rendertarget) {
    delete reinterpret_cast<GrBackendRenderTarget*>(rendertarget);
}

gr_direct_context_t* gr_direct_context_make_gl(const gr_glinterface_t* glInterface) {
    return reinterpret_cast<gr_direct_context_t *>(GrDirectContexts::MakeGL(sk_ref_sp(reinterpret_cast<const GrGLInterface*>(glInterface))).release());
}

// ===== Functions from include/gpu/GrDirectContext.h =====
void gr_direct_context_abandon_context(gr_direct_context_t* context) {
    reinterpret_cast<GrDirectContext*>(context)->abandonContext();
}

void gr_direct_context_delete(gr_direct_context_t* context) {
    delete reinterpret_cast<GrDirectContext*>(context);
}

void gr_direct_context_flush_and_submit(gr_direct_context_t* context, bool syncCPU) {
    reinterpret_cast<GrDirectContext*>(context)->flushAndSubmit(syncCPU ? GrSyncCpu::kYes : GrSyncCpu::kNo);
}

void gr_direct_context_release_resources_and_abandon_context(gr_direct_context_t* context) {
    reinterpret_cast<GrDirectContext*>(context)->releaseResourcesAndAbandonContext();
}

void gr_direct_context_reset(gr_direct_context_t* context) {
    reinterpret_cast<GrDirectContext*>(context)->resetContext();
}

void gr_direct_context_reset_gl_texture_bindings(gr_direct_context_t* context) {
    reinterpret_cast<GrDirectContext*>(context)->resetGLTextureBindings();
}

void gr_direct_context_unref(const gr_direct_context_t* context) {
    SkSafeUnref(reinterpret_cast<const GrDirectContext*>(context));
}

// ===== Functions from include/gpu/gl/GrGLInterface.h =====
const gr_glinterface_t* gr_glinterface_create_native_interface(void) {
    return reinterpret_cast<const gr_glinterface_t*>(GrGLMakeNativeInterface().release());
}

void gr_glinterface_unref(const gr_glinterface_t* intf) {
    SkSafeUnref(reinterpret_cast<const GrGLInterface*>(intf));
}

// ===== Functions from include/core/SkCanvas.h =====
sk_surface_t* sk_canvas_get_surface(sk_canvas_t* canvas) {
    return reinterpret_cast<sk_surface_t*>(reinterpret_cast<SkCanvas*>(canvas)->getSurface());
}

void sk_canvas_clear(sk_canvas_t* canvas, sk_color_t color) {
    reinterpret_cast<SkCanvas*>(canvas)->clear(color);
}

void sk_canvas_clip_path_with_operation(sk_canvas_t* canvas, const sk_path_t* cpath, sk_clip_op_t op, bool doAA) {
    reinterpret_cast<SkCanvas*>(canvas)->clipPath(*reinterpret_cast<const SkPath*>(cpath), (SkClipOp)op, doAA);
}

void sk_canvas_clip_rect_with_operation(sk_canvas_t* canvas, const sk_rect_t* crect, sk_clip_op_t op, bool doAA) {
    reinterpret_cast<SkCanvas*>(canvas)->clipRect(*reinterpret_cast<const SkRect*>(crect), (SkClipOp)op, doAA);
}

void sk_canvas_concat(sk_canvas_t* canvas, const sk_matrix_t* matrix) {
    reinterpret_cast<SkCanvas*>(canvas)->concat(AsMatrix(matrix));
}

void sk_canvas_draw_arc(sk_canvas_t* canvas, const sk_rect_t* oval, float startAngle, float sweepAngle, bool useCenter, const sk_paint_t* paint) {
    reinterpret_cast<SkCanvas*>(canvas)->drawArc(*reinterpret_cast<const SkRect*>(oval), startAngle, sweepAngle, useCenter, *reinterpret_cast<const SkPaint *>(paint));
}

void sk_canvas_draw_circle(sk_canvas_t* canvas, float cx, float cy, float rad, const sk_paint_t* cpaint) {
    reinterpret_cast<SkCanvas*>(canvas)->drawCircle(cx, cy, rad, *reinterpret_cast<const SkPaint*>(cpaint));
}

void sk_canvas_draw_color(sk_canvas_t* canvas, sk_color_t color, sk_blend_mode_t cmode) {
    reinterpret_cast<SkCanvas*>(canvas)->drawColor(color, (SkBlendMode)cmode);
}

void sk_canvas_draw_image_nine(sk_canvas_t* canvas, const sk_image_t* image, const sk_irect_t* center, const sk_rect_t* dst, sk_filter_mode_t filter, const sk_paint_t* paint) {
    reinterpret_cast<SkCanvas*>(canvas)->drawImageNine(reinterpret_cast<const SkImage*>(image), *reinterpret_cast<const SkIRect*>(center), *reinterpret_cast<const SkRect*>(dst), (SkFilterMode)filter, reinterpret_cast<const SkPaint*>(paint));
}

void sk_canvas_draw_image_rect(sk_canvas_t* canvas, const sk_image_t* cimage, const sk_rect_t* csrcR, const sk_rect_t* cdstR, const sk_sampling_options_t *samplingOptions, const sk_paint_t* cpaint, sk_src_rect_constraint_t constraint) {
    if (csrcR) {
        reinterpret_cast<SkCanvas*>(canvas)->drawImageRect(reinterpret_cast<const SkImage*>(cimage), *reinterpret_cast<const SkRect*>(csrcR), *reinterpret_cast<const SkRect*>(cdstR), *reinterpret_cast<const SkSamplingOptions*>(samplingOptions), reinterpret_cast<const SkPaint*>(cpaint), (SkCanvas::SrcRectConstraint)constraint);
    } else {
        reinterpret_cast<SkCanvas*>(canvas)->drawImageRect(reinterpret_cast<const SkImage*>(cimage), *reinterpret_cast<const SkRect*>(cdstR), *reinterpret_cast<const SkSamplingOptions*>(samplingOptions), reinterpret_cast<const SkPaint*>(cpaint));
    }
}

void sk_canvas_draw_line(sk_canvas_t* canvas, float x0, float y0, float x1, float y1, const sk_paint_t* cpaint) {
    reinterpret_cast<SkCanvas*>(canvas)->drawLine(x0, y0, x1, y1, *reinterpret_cast<const SkPaint*>(cpaint));
}

void sk_canvas_draw_oval(sk_canvas_t* canvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    reinterpret_cast<SkCanvas*>(canvas)->drawOval(*reinterpret_cast<const SkRect*>(crect), *reinterpret_cast<const SkPaint*>(cpaint));
}

void sk_canvas_draw_paint(sk_canvas_t* canvas, const sk_paint_t* cpaint) {
    reinterpret_cast<SkCanvas*>(canvas)->drawPaint(*reinterpret_cast<const SkPaint*>(cpaint));
}

void sk_canvas_draw_path(sk_canvas_t* canvas, const sk_path_t* cpath, const sk_paint_t* cpaint) {
    reinterpret_cast<SkCanvas*>(canvas)->drawPath(*reinterpret_cast<const SkPath*>(cpath), *reinterpret_cast<const SkPaint*>(cpaint));
}

void sk_canvas_draw_point(sk_canvas_t* canvas, float x, float y, const sk_paint_t* cpaint) {
    reinterpret_cast<SkCanvas*>(canvas)->drawPoint (x, y, *reinterpret_cast<const SkPaint*>(cpaint));
}

void sk_canvas_draw_points(sk_canvas_t* canvas, sk_point_mode_t pointMode, size_t count, const sk_point_t points[], const sk_paint_t* cpaint) {
    reinterpret_cast<SkCanvas*>(canvas)->drawPoints ((SkCanvas::PointMode)pointMode, count, reinterpret_cast<const SkPoint*>(points), *reinterpret_cast<const SkPaint*>(cpaint));
}

void sk_canvas_draw_rect(sk_canvas_t* canvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    reinterpret_cast<SkCanvas*>(canvas)->drawRect(*reinterpret_cast<const SkRect*>(crect), *reinterpret_cast<const SkPaint*>(cpaint));
}

void sk_canvas_draw_round_rect(sk_canvas_t* canvas, const sk_rect_t* crect, float rx, float ry, const sk_paint_t* cpaint) {
    reinterpret_cast<SkCanvas*>(canvas)->drawRoundRect(*reinterpret_cast<const SkRect*>(crect), rx, ry, *reinterpret_cast<const SkPaint*>(cpaint));
}

void sk_canvas_draw_simple_text(sk_canvas_t* canvas, const void* text, size_t byte_length, sk_text_encoding_t encoding, float x, float y, const sk_font_t* cfont, const sk_paint_t* cpaint) {
    reinterpret_cast<SkCanvas*>(canvas)->drawSimpleText(text, byte_length, (SkTextEncoding)encoding, x, y, *reinterpret_cast<const SkFont*>(cfont), *reinterpret_cast<const SkPaint*>(cpaint));
}

void sk_canvas_draw_text_blob (sk_canvas_t* canvas, sk_text_blob_t* text, float x, float y, const sk_paint_t* cpaint) {
    reinterpret_cast<SkCanvas*>(canvas)->drawTextBlob(reinterpret_cast<SkTextBlob*>(text), x, y, *reinterpret_cast<const SkPaint*>(cpaint));
}

bool sk_canvas_get_local_clip_bounds(sk_canvas_t* canvas, sk_rect_t* cbounds) {
    return reinterpret_cast<SkCanvas*>(canvas)->getLocalClipBounds(reinterpret_cast<SkRect*>(cbounds));
}

int sk_canvas_get_save_count(sk_canvas_t* canvas) {
    return reinterpret_cast<SkCanvas*>(canvas)->getSaveCount();
}

void sk_canvas_get_total_matrix(sk_canvas_t* canvas, sk_matrix_t* cmatrix) {
    const SkMatrix matrix = reinterpret_cast<SkCanvas*>(canvas)->getTotalMatrix();
    *cmatrix = ToMatrix(&matrix);
}

bool sk_canvas_is_clip_empty(sk_canvas_t* canvas) {
    return reinterpret_cast<SkCanvas*>(canvas)->isClipEmpty();
}

bool sk_canvas_is_clip_rect(sk_canvas_t* canvas) {
    return reinterpret_cast<SkCanvas*>(canvas)->isClipRect();
}

bool sk_canvas_quick_reject_path(sk_canvas_t* canvas, const sk_path_t* path) {
    return reinterpret_cast<SkCanvas*>(canvas)->quickReject(*reinterpret_cast<const SkPath*>(path));
}

bool sk_canvas_quick_reject_rect(sk_canvas_t* canvas, const sk_rect_t* rect) {
    return reinterpret_cast<SkCanvas*>(canvas)->quickReject(*reinterpret_cast<const SkRect*>(rect));
}

void sk_canvas_reset_matrix(sk_canvas_t* canvas) {
    reinterpret_cast<SkCanvas*>(canvas)->resetMatrix();
}

void sk_canvas_restore(sk_canvas_t* canvas) {
    reinterpret_cast<SkCanvas*>(canvas)->restore();
}

void sk_canvas_restore_to_count(sk_canvas_t* canvas, int saveCount) {
    reinterpret_cast<SkCanvas*>(canvas)->restoreToCount(saveCount);
}

void sk_canvas_rotate_radians(sk_canvas_t* canvas, float radians) {
    reinterpret_cast<SkCanvas*>(canvas)->rotate(SkRadiansToDegrees(radians));
}

int sk_canvas_save(sk_canvas_t* canvas) {
    return reinterpret_cast<SkCanvas*>(canvas)->save();
}

int sk_canvas_save_layer(sk_canvas_t* canvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    return reinterpret_cast<SkCanvas*>(canvas)->saveLayer(reinterpret_cast<const SkRect*>(crect), reinterpret_cast<const SkPaint*>(cpaint));
}

int sk_canvas_save_layer_alpha(sk_canvas_t* canvas, const sk_rect_t* crect, uint8_t alpha) {
    return reinterpret_cast<SkCanvas*>(canvas)->saveLayerAlpha(reinterpret_cast<const SkRect*>(crect), alpha);
}
void sk_canvas_scale(sk_canvas_t* canvas, float sx, float sy) {
    reinterpret_cast<SkCanvas*>(canvas)->scale(sx, sy);
}

void sk_canvas_set_matrix(sk_canvas_t* canvas, const sk_matrix_t* cmatrix) {
    reinterpret_cast<SkCanvas*>(canvas)->setMatrix(AsMatrix(cmatrix));
}

void sk_canvas_skew(sk_canvas_t* canvas, float sx, float sy) {
    reinterpret_cast<SkCanvas*>(canvas)->skew(sx, sy);
}

void sk_canvas_translate(sk_canvas_t* canvas, float dx, float dy) {
    reinterpret_cast<SkCanvas*>(canvas)->translate(dx, dy);
}

// ===== Functions from include/core/SkColorFilter.h =====
sk_color_filter_t* sk_colorfilter_new_color_matrix(const float array[20]) {
    return reinterpret_cast<sk_color_filter_t*>(SkColorFilters::Matrix(array).release());
}

sk_color_filter_t* sk_colorfilter_new_compose(sk_color_filter_t* outer, sk_color_filter_t* inner) {
    return reinterpret_cast<sk_color_filter_t*>(SkColorFilters::Compose(sk_ref_sp(reinterpret_cast<SkColorFilter*>(outer)), sk_ref_sp(reinterpret_cast<SkColorFilter*>(inner))).release());
}

sk_color_filter_t* sk_colorfilter_new_high_contrast(const sk_high_contrast_config_t* config) {
    return reinterpret_cast<sk_color_filter_t*>(SkHighContrastFilter::Make(*reinterpret_cast<const SkHighContrastConfig*>(config)).release());
}

sk_color_filter_t* sk_colorfilter_new_lighting(sk_color_t mul, sk_color_t add) {
    return reinterpret_cast<sk_color_filter_t*>(SkColorMatrixFilter::MakeLightingFilter(mul, add).release());
}

sk_color_filter_t* sk_colorfilter_new_luma_color(void) {
    return reinterpret_cast<sk_color_filter_t*>(SkLumaColorFilter::Make().release());
}

sk_color_filter_t* sk_colorfilter_new_mode(sk_color_t c, sk_blend_mode_t cmode) {
    return reinterpret_cast<sk_color_filter_t*>(SkColorFilters::Blend(c, (SkBlendMode)cmode).release());
}

void sk_colorfilter_unref(sk_color_filter_t* filter) {
    SkSafeUnref(reinterpret_cast<SkColorFilter*>(filter));
}

// ===== Functions from include/core/SkColorSpace.h =====
sk_color_space_t* sk_colorspace_new_srgb(void) {
    return reinterpret_cast<sk_color_space_t*>(SkColorSpace::MakeSRGB().release());
}

// ===== Functions from include/core/SkData.h =====
const void* sk_data_get_data(const sk_data_t* cdata) {
    return reinterpret_cast<const SkData*>(cdata)->data();
}

size_t sk_data_get_size(const sk_data_t* cdata) {
    return reinterpret_cast<const SkData*>(cdata)->size();
}

sk_data_t* sk_data_new_with_copy(const void* src, size_t length) {
    return reinterpret_cast<sk_data_t*>(SkData::MakeWithCopy(src, length).release());
}

void sk_data_unref(const sk_data_t* cdata) {
    SkSafeUnref(reinterpret_cast<const SkData*>(cdata));
}

// ===== Functions from include/encode/SkJpegEncoder.h =====
sk_data_t* sk_encode_jpeg(gr_direct_context_t* ctx, const sk_image_t* img, int quality) {
    SkJpegEncoder::Options options;
    options.fQuality = quality;
    return reinterpret_cast<sk_data_t*>(SkJpegEncoder::Encode(reinterpret_cast<GrDirectContext*>(ctx), reinterpret_cast<const SkImage*>(img), options).release());
}

// ===== Functions from include/encode/SkPngEncoder.h =====
sk_data_t* sk_encode_png(gr_direct_context_t* ctx, const sk_image_t* img, int compressionLevel) {
    SkPngEncoder::Options options;
    options.fFilterFlags = SkPngEncoder::FilterFlag::kAll;
    options.fZLibLevel = compressionLevel;
    return reinterpret_cast<sk_data_t*>(SkPngEncoder::Encode(reinterpret_cast<GrDirectContext*>(ctx), reinterpret_cast<const SkImage*>(img), options).release());
}

// ===== Functions from include/encode/SkWebpEncoder.h =====
sk_data_t* sk_encode_webp(gr_direct_context_t* ctx, const sk_image_t* img, float quality, bool lossy) {
    SkWebpEncoder::Options options;
    options.fCompression = lossy ? SkWebpEncoder::Compression::kLossy : SkWebpEncoder::Compression::kLossless;
    options.fQuality = quality;
    return reinterpret_cast<sk_data_t*>(SkWebpEncoder::Encode(reinterpret_cast<GrDirectContext*>(ctx), reinterpret_cast<const SkImage*>(img), options).release());
}

// ===== Functions from include/core/SkFont.h =====
void sk_font_delete(sk_font_t* font) {
    delete reinterpret_cast<SkFont*>(font);
}

float sk_font_get_metrics(const sk_font_t* font, sk_font_metrics_t* metrics) {
    return reinterpret_cast<const SkFont*>(font)->getMetrics(reinterpret_cast<SkFontMetrics*>(metrics));
}

void sk_font_get_xpos(const sk_font_t* font, const uint16_t glyphs[], int count, float xpos[], float origin) {
    reinterpret_cast<const SkFont*>(font)->getXPos(glyphs, count, xpos, origin);
}

float sk_font_measure_text(const sk_font_t* font, const void* text, size_t byteLength, sk_text_encoding_t encoding, sk_rect_t* bounds, const sk_paint_t* paint) {
    return reinterpret_cast<const SkFont*>(font)->measureText(text, byteLength, (SkTextEncoding)encoding, reinterpret_cast<SkRect*>(bounds), reinterpret_cast<const SkPaint*>(paint));
}

sk_font_t* sk_font_new_with_values(sk_typeface_t* typeface, float size, float scaleX, float skewX) {
    return reinterpret_cast<sk_font_t*>(new SkFont(sk_ref_sp(reinterpret_cast<SkTypeface*>(typeface)), size, scaleX, skewX));
}

void sk_font_set_force_auto_hinting(sk_font_t* font, bool value) {
    reinterpret_cast<SkFont*>(font)->setForceAutoHinting(value);
}

void sk_font_set_hinting(sk_font_t* font, sk_font_hinting_t value) {
    reinterpret_cast<SkFont*>(font)->setHinting((SkFontHinting)value);
}

void sk_font_set_subpixel(sk_font_t* font, bool value) {
    reinterpret_cast<SkFont*>(font)->setSubpixel(value);
}

int sk_font_text_to_glyphs(const sk_font_t* font, const void* text, size_t byteLength, sk_text_encoding_t encoding, uint16_t glyphs[], int maxGlyphCount) {
    return reinterpret_cast<const SkFont*>(font)->textToGlyphs(text, byteLength, (SkTextEncoding)encoding, glyphs, maxGlyphCount);
}

uint16_t sk_font_unichar_to_glyph(const sk_font_t* font, int32_t unichar) {
    return reinterpret_cast<const SkFont*>(font)->unicharToGlyph(unichar);
}

void sk_font_unichars_to_glyphs(const sk_font_t* font, const int32_t* unichars, int count, uint16_t* glyphs) {
    return reinterpret_cast<const SkFont*>(font)->unicharsToGlyphs(unichars, count, glyphs);
}

void sk_font_glyph_widths(const sk_font_t* font, const uint16_t *glyphs, int count, float *widths) {
    return reinterpret_cast<const SkFont*>(font)->getWidths(glyphs, count, widths);
}

// ===== Functions from include/core/SkFontMgr.h =====
int sk_fontmgr_count_families(sk_font_mgr_t* fontmgr) {
    return reinterpret_cast<SkFontMgr*>(fontmgr)->countFamilies();
}

sk_typeface_t* sk_fontmgr_create_from_data(sk_font_mgr_t* fontmgr, sk_data_t* data, int index) {
    return reinterpret_cast<sk_typeface_t*>(reinterpret_cast<SkFontMgr*>(fontmgr)->makeFromData(sk_ref_sp(reinterpret_cast<SkData*>(data)), index).release());
}

void sk_fontmgr_get_family_name(sk_font_mgr_t* fontmgr, int index, sk_string_t* familyName) {
    reinterpret_cast<SkFontMgr*>(fontmgr)->getFamilyName(index, reinterpret_cast<SkString*>(familyName));
}

sk_font_style_set_t* sk_fontmgr_match_family(sk_font_mgr_t* fontmgr, const char* familyName) {
    return reinterpret_cast<sk_font_style_set_t*>(reinterpret_cast<SkFontMgr*>(fontmgr)->matchFamily(familyName).release());
}

sk_typeface_t* sk_fontmgr_match_family_style(sk_font_mgr_t* fontmgr, const char* familyName, sk_font_style_t* style) {
    return reinterpret_cast<sk_typeface_t*>(reinterpret_cast<SkFontMgr*>(fontmgr)->matchFamilyStyle(familyName, *reinterpret_cast<SkFontStyle*>(style)).release());
}

sk_typeface_t* sk_fontmgr_match_family_style_character(sk_font_mgr_t* fontmgr, const char familyName[], sk_font_style_t* style, const char** bcp47, int bcp47Count, int32_t character) {
    return reinterpret_cast<sk_typeface_t*>(reinterpret_cast<SkFontMgr*>(fontmgr)->matchFamilyStyleCharacter(familyName, *reinterpret_cast<SkFontStyle*>(style), bcp47, bcp47Count, character).release());
}

sk_font_mgr_t* sk_fontmgr_ref_default(void) {
#if defined(SK_BUILD_FOR_MAC)
	return reinterpret_cast<sk_font_mgr_t*>(SkFontMgr_New_CoreText(nullptr).release());
#elif defined(SK_BUILD_FOR_WIN)
	return reinterpret_cast<sk_font_mgr_t*>(SkFontMgr_New_DirectWrite().release());
#elif defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
	return reinterpret_cast<sk_font_mgr_t*>(SkFontMgr_New_FontConfig(nullptr).release());
#else
	#error "No font manager available for this platform"
#endif
}

sk_typeface_t* sk_fontstyleset_create_typeface(sk_font_style_set_t* fss, int index) {
    return reinterpret_cast<sk_typeface_t*>(reinterpret_cast<SkFontStyleSet*>(fss)->createTypeface(index).release());
}

int sk_fontstyleset_get_count(sk_font_style_set_t* fss) {
    return reinterpret_cast<SkFontStyleSet*>(fss)->count();
}

void sk_fontstyleset_get_style(sk_font_style_set_t* fss, int index, sk_font_style_t* fs, sk_string_t* style) {
    return reinterpret_cast<SkFontStyleSet*>(fss)->getStyle(index, reinterpret_cast<SkFontStyle*>(fs), reinterpret_cast<SkString*>(style));
}

sk_typeface_t* sk_fontstyleset_match_style(sk_font_style_set_t* fss, sk_font_style_t* style) {
    return reinterpret_cast<sk_typeface_t*>(reinterpret_cast<SkFontStyleSet*>(fss)->matchStyle(*reinterpret_cast<SkFontStyle*>(style)).release());
}

void sk_fontstyleset_unref(sk_font_style_set_t* fss) {
    reinterpret_cast<SkFontStyleSet*>(fss)->unref();
}

// ===== Functions from include/core/SkFontStyle.h =====
void sk_fontstyle_delete(sk_font_style_t* fs) {
    delete reinterpret_cast<SkFontStyle*>(fs);
}

sk_font_style_slant_t sk_fontstyle_get_slant(const sk_font_style_t* fs) {
    return (sk_font_style_slant_t)reinterpret_cast<const SkFontStyle*>(fs)->slant();
}

int sk_fontstyle_get_weight(const sk_font_style_t* fs) {
    return reinterpret_cast<const SkFontStyle*>(fs)->weight();
}

int sk_fontstyle_get_width(const sk_font_style_t* fs) {
    return reinterpret_cast<const SkFontStyle*>(fs)->width();
}

sk_font_style_t* sk_fontstyle_new(int weight, int width, sk_font_style_slant_t slant) {
    return reinterpret_cast<sk_font_style_t*>(new SkFontStyle(weight, width,(SkFontStyle::Slant)slant));
}

// ===== Functions from include/core/SkImage.h =====
sk_alpha_type_t sk_image_get_alpha_type(const sk_image_t* image) {
    return (sk_alpha_type_t)reinterpret_cast<const SkImage*>(image)->alphaType();
}

sk_color_type_t sk_image_get_color_type(const sk_image_t* image) {
    return (sk_color_type_t)reinterpret_cast<const SkImage*>(image)->colorType();
}

sk_color_space_t* sk_image_get_colorspace(const sk_image_t* image) {
    return reinterpret_cast<sk_color_space_t*>(reinterpret_cast<const SkImage*>(image)->refColorSpace().release());
}

int sk_image_get_height(const sk_image_t* cimage) {
    return reinterpret_cast<const SkImage*>(cimage)->height();
}

int sk_image_get_width(const sk_image_t* cimage) {
    return reinterpret_cast<const SkImage*>(cimage)->width();
}

sk_image_t* sk_image_make_non_texture_image(const sk_image_t* image) {
    return reinterpret_cast<sk_image_t*>(reinterpret_cast<const SkImage*>(image)->makeNonTextureImage().release());
}

sk_shader_t* sk_image_make_shader(const sk_image_t* image, sk_tile_mode_t tileX, sk_tile_mode_t tileY, const sk_sampling_options_t *samplingOptions, const sk_matrix_t* cmatrix) {
    SkMatrix m;
    if (cmatrix) {
        m = AsMatrix(cmatrix);
    }
    return reinterpret_cast<sk_shader_t*>(reinterpret_cast<const SkImage*>(image)->makeShader((SkTileMode)tileX, (SkTileMode)tileY, *reinterpret_cast<const SkSamplingOptions*>(samplingOptions), cmatrix ? &m : nullptr).release());
}

sk_image_t* sk_image_new_from_encoded(sk_data_t* cdata) {
    return reinterpret_cast<sk_image_t*>(SkImages::DeferredFromEncodedData(sk_ref_sp(reinterpret_cast<SkData*>(cdata)), std::nullopt).release());
}

sk_image_t* sk_image_new_raster_data(const sk_image_info_t* cinfo, sk_data_t* pixels, size_t rowBytes) {
    return reinterpret_cast<sk_image_t*>(SkImages::RasterFromData(AsImageInfo(cinfo), sk_ref_sp(reinterpret_cast<SkData*>(pixels)), rowBytes).release());
}

bool sk_image_read_pixels(const sk_image_t* image, const sk_image_info_t* dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY, sk_image_caching_hint_t cachingHint) {
    return reinterpret_cast<const SkImage*>(image)->readPixels(AsImageInfo(dstInfo), dstPixels, dstRowBytes, srcX, srcY, (SkImage::CachingHint)cachingHint);
}

void sk_image_unref(const sk_image_t* cimage) {
    SkSafeUnref(reinterpret_cast<const SkImage*>(cimage));
}

// ===== Functions from include/gpu/ganesh/SkImageGanesh.h =====
sk_image_t* sk_image_texture_from_image(gr_direct_context_t* ctx, const sk_image_t* image, bool mipmapped, bool budgeted) {
    return reinterpret_cast<sk_image_t*>(SkImages::TextureFromImage(reinterpret_cast<GrDirectContext*>(ctx),
        reinterpret_cast<const SkImage*>(image), (skgpu::Mipmapped)mipmapped, (skgpu::Budgeted)budgeted).release());
}

// ===== Functions from include/core/SkImageFilter.h =====
sk_image_filter_t* sk_imagefilter_new_arithmetic(float k1, float k2, float k3, float k4, bool enforcePMColor, sk_image_filter_t* background, sk_image_filter_t* foreground, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::Arithmetic(k1, k2, k3, k4, enforcePMColor,
        sk_ref_sp(reinterpret_cast<SkImageFilter*>(background)), sk_ref_sp(reinterpret_cast<SkImageFilter*>(foreground)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_blur(float sigmaX, float sigmaY, sk_tile_mode_t tileMode, sk_image_filter_t* input, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::Blur(sigmaX, sigmaY,
        (SkTileMode)tileMode, sk_ref_sp(reinterpret_cast<SkImageFilter*>(input)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_color_filter(sk_color_filter_t* cf, sk_image_filter_t* input, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::ColorFilter(sk_ref_sp(reinterpret_cast<SkColorFilter*>(cf)),
        sk_ref_sp(reinterpret_cast<SkImageFilter*>(input)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_compose(sk_image_filter_t* outer, sk_image_filter_t* inner) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::Compose(sk_ref_sp(reinterpret_cast<SkImageFilter*>(outer)), sk_ref_sp(reinterpret_cast<SkImageFilter*>(inner))).release());
}

sk_image_filter_t* sk_imagefilter_new_dilate(int radiusX, int radiusY, sk_image_filter_t* input, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::Dilate(radiusX, radiusY,
        sk_ref_sp(reinterpret_cast<SkImageFilter*>(input)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_displacement_map_effect(sk_color_channel_t xChannelSelector, sk_color_channel_t yChannelSelector, float scale, sk_image_filter_t* displacement, sk_image_filter_t* color, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::DisplacementMap((SkColorChannel)xChannelSelector,
        (SkColorChannel)yChannelSelector, scale, sk_ref_sp(reinterpret_cast<SkImageFilter*>(displacement)),
        sk_ref_sp(reinterpret_cast<SkImageFilter*>(color)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_distant_lit_diffuse(const sk_point3_t* direction, sk_color_t lightColor, float surfaceScale, float kd, sk_image_filter_t* input, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::DistantLitDiffuse(
        *reinterpret_cast<const SkPoint3*>(direction), lightColor, surfaceScale, kd,
        sk_ref_sp(reinterpret_cast<SkImageFilter*>(input)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_distant_lit_specular(const sk_point3_t* direction, sk_color_t lightColor, float surfaceScale, float ks, float shininess, sk_image_filter_t* input, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::DistantLitSpecular(
        *reinterpret_cast<const SkPoint3*>(direction), lightColor, surfaceScale, ks, shininess,
        sk_ref_sp(reinterpret_cast<SkImageFilter*>(input)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_drop_shadow(float dx, float dy, float sigmaX, float sigmaY, sk_color_t color, sk_image_filter_t* input, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::DropShadow(dx, dy, sigmaX, sigmaY, color,
        sk_ref_sp(reinterpret_cast<SkImageFilter*>(input)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_drop_shadow_only(float dx, float dy, float sigmaX, float sigmaY, sk_color_t color, sk_image_filter_t* input, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::DropShadowOnly(dx, dy, sigmaX, sigmaY, color,
        sk_ref_sp(reinterpret_cast<SkImageFilter*>(input)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_erode(int radiusX, int radiusY, sk_image_filter_t* input, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::Erode(radiusX, radiusY,
        sk_ref_sp(reinterpret_cast<SkImageFilter*>(input)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_image_source(sk_image_t* image, const sk_rect_t* srcRect, const sk_rect_t* dstRect, const sk_sampling_options_t* samplingOptions) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::Image(sk_ref_sp(reinterpret_cast<SkImage*>(image)),
        *reinterpret_cast<const SkRect*>(srcRect), *reinterpret_cast<const SkRect*>(dstRect),
        *reinterpret_cast<const SkSamplingOptions*>(samplingOptions)).release());
}

sk_image_filter_t* sk_imagefilter_new_image_source_default(sk_image_t* image, const sk_sampling_options_t* samplingOptions) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::Image(sk_ref_sp(reinterpret_cast<SkImage*>(image)),
        *reinterpret_cast<const SkSamplingOptions*>(samplingOptions)).release());
}

sk_image_filter_t* sk_imagefilter_new_magnifier(const sk_rect_t* lensBounds, float zoomAmount, float inset, const sk_sampling_options_t* samplingOptions, sk_image_filter_t* input, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::Magnifier(*reinterpret_cast<const SkRect*>(lensBounds),
        zoomAmount, inset, *reinterpret_cast<const SkSamplingOptions*>(samplingOptions),
        sk_ref_sp(reinterpret_cast<SkImageFilter*>(input)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_matrix_convolution(const sk_isize_t* kernelSize, const float kernel[], float gain, float bias, const sk_ipoint_t* kernelOffset, sk_tile_mode_t ctileMode, bool convolveAlpha, sk_image_filter_t* input, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::MatrixConvolution(
        *reinterpret_cast<const SkISize*>(kernelSize), kernel, gain, bias,
        *reinterpret_cast<const SkIPoint*>(kernelOffset), (SkTileMode)ctileMode, convolveAlpha,
        sk_ref_sp(reinterpret_cast<SkImageFilter*>(input)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_matrix_transform(const sk_matrix_t* matrix, const sk_sampling_options_t *samplingOptions, sk_image_filter_t* input) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::MatrixTransform(AsMatrix(matrix),
        *reinterpret_cast<const SkSamplingOptions*>(samplingOptions),
        sk_ref_sp(reinterpret_cast<SkImageFilter*>(input))).release());
}

sk_image_filter_t* sk_imagefilter_new_merge(sk_image_filter_t* cfilters[], int count, const sk_rect_t* cropRect) {
    std::vector<sk_sp<SkImageFilter>> filters(count);
    for (int i = 0; i < count; i++) {
        filters[i] = sk_ref_sp(reinterpret_cast<SkImageFilter*>(cfilters[i]));
    }
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::Merge(filters.data(), count,
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_offset(float dx, float dy, sk_image_filter_t* input, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::Offset(dx, dy,
        sk_ref_sp(reinterpret_cast<SkImageFilter*>(input)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_point_lit_diffuse(const sk_point3_t* location, sk_color_t lightColor, float surfaceScale, float kd, sk_image_filter_t* input, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::PointLitDiffuse(
        *reinterpret_cast<const SkPoint3*>(location), lightColor, surfaceScale, kd,
        sk_ref_sp(reinterpret_cast<SkImageFilter*>(input)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_point_lit_specular(const sk_point3_t* location, sk_color_t lightColor, float surfaceScale, float ks, float shininess, sk_image_filter_t* input, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::PointLitSpecular(
        *reinterpret_cast<const SkPoint3*>(location), lightColor, surfaceScale, ks, shininess,
        sk_ref_sp(reinterpret_cast<SkImageFilter*>(input)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_spot_lit_diffuse(const sk_point3_t* location, const sk_point3_t* target, float specularExponent, float cutoffAngle, sk_color_t lightColor, float surfaceScale, float kd, sk_image_filter_t* input, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::SpotLitDiffuse(
        *reinterpret_cast<const SkPoint3*>(location), *reinterpret_cast<const SkPoint3*>(target), specularExponent,
        cutoffAngle, lightColor, surfaceScale, kd, sk_ref_sp(reinterpret_cast<SkImageFilter*>(input)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_spot_lit_specular(const sk_point3_t* location, const sk_point3_t* target, float specularExponent, float cutoffAngle, sk_color_t lightColor, float surfaceScale, float ks, float shininess, sk_image_filter_t* input, const sk_rect_t* cropRect) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::SpotLitSpecular(
        *reinterpret_cast<const SkPoint3*>(location), *reinterpret_cast<const SkPoint3*>(target), specularExponent,
        cutoffAngle, lightColor, surfaceScale, ks, shininess, sk_ref_sp(reinterpret_cast<SkImageFilter*>(input)),
        SkImageFilters::CropRect(reinterpret_cast<const SkRect*>(cropRect))).release());
}

sk_image_filter_t* sk_imagefilter_new_tile(const sk_rect_t* src, const sk_rect_t* dst, sk_image_filter_t* input) {
    return reinterpret_cast<sk_image_filter_t*>(SkImageFilters::Tile(*reinterpret_cast<const SkRect*>(src),
        *reinterpret_cast<const SkRect*>(dst), sk_ref_sp(reinterpret_cast<SkImageFilter*>(input))).release());
}

void sk_imagefilter_unref(sk_image_filter_t* filter) {
    SkSafeUnref(reinterpret_cast<SkImageFilter*>(filter));
}

// ===== Functions from include/core/SkMaskFilter.h =====
sk_mask_filter_t* sk_maskfilter_new_blur_with_flags(sk_blur_style_t cstyle, float sigma, bool respectCTM) {
    return reinterpret_cast<sk_mask_filter_t*>(SkMaskFilter::MakeBlur((SkBlurStyle)cstyle, sigma, respectCTM).release());
}

sk_mask_filter_t* sk_maskfilter_new_clip(uint8_t min, uint8_t max) {
    return reinterpret_cast<sk_mask_filter_t*>(SkTableMaskFilter::CreateClip(min, max));
}

sk_mask_filter_t* sk_maskfilter_new_gamma(float gamma) {
    return reinterpret_cast<sk_mask_filter_t*>(SkTableMaskFilter::CreateGamma(gamma));
}

sk_mask_filter_t* sk_maskfilter_new_shader(sk_shader_t* cshader) {
    return reinterpret_cast<sk_mask_filter_t*>(SkShaderMaskFilter::Make(sk_ref_sp(reinterpret_cast<SkShader*>(cshader))).release());
}

sk_mask_filter_t* sk_maskfilter_new_table(const uint8_t table[256]) {
    return reinterpret_cast<sk_mask_filter_t*>(SkTableMaskFilter::Create(table));
}

void sk_maskfilter_unref(sk_mask_filter_t* filter) {
    SkSafeUnref(reinterpret_cast<SkMaskFilter*>(filter));
}

// ===== Functions from include/core/SkPaint.h =====

bool sk_paint_equivalent(sk_paint_t* cpaint, sk_paint_t* other) {
    return reinterpret_cast<SkPaint*>(cpaint) == reinterpret_cast<SkPaint*>(other);
}

sk_paint_t* sk_paint_clone(sk_paint_t* paint) {
    return reinterpret_cast<sk_paint_t*>(new SkPaint(*reinterpret_cast<SkPaint*>(paint)));
}

void sk_paint_delete(sk_paint_t* cpaint) {
    delete reinterpret_cast<SkPaint*>(cpaint);
}

sk_blend_mode_t sk_paint_get_blend_mode_or(sk_paint_t* paint, sk_blend_mode_t defaultMode) {
    return (sk_blend_mode_t)reinterpret_cast<SkPaint*>(paint)->getBlendMode_or((SkBlendMode)defaultMode);
}

sk_color_t sk_paint_get_color(const sk_paint_t* cpaint) {
    return reinterpret_cast<const SkPaint*>(cpaint)->getColor();
}

sk_color_filter_t* sk_paint_get_colorfilter(sk_paint_t* cpaint) {
    return reinterpret_cast<sk_color_filter_t*>(reinterpret_cast<SkPaint*>(cpaint)->refColorFilter().release());
}

bool sk_paint_get_fill_path(const sk_paint_t* cpaint, const sk_path_t* src, sk_path_t* dst, const sk_rect_t* cullRect, float resScale) {
    return skpathutils::FillPathWithPaint(*reinterpret_cast<const SkPath*>(src), *reinterpret_cast<const SkPaint*>(cpaint), reinterpret_cast<SkPath*>(dst), reinterpret_cast<const SkRect*>(cullRect), resScale);
}

sk_image_filter_t* sk_paint_get_imagefilter(sk_paint_t* cpaint) {
    return reinterpret_cast<sk_image_filter_t*>(reinterpret_cast<SkPaint*>(cpaint)->refImageFilter().release());
}

sk_mask_filter_t* sk_paint_get_maskfilter(sk_paint_t* cpaint) {
    return reinterpret_cast<sk_mask_filter_t*>(reinterpret_cast<SkPaint*>(cpaint)->refMaskFilter().release());
}

sk_path_effect_t* sk_paint_get_path_effect(sk_paint_t* cpaint) {
    return reinterpret_cast<sk_path_effect_t*>(reinterpret_cast<SkPaint*>(cpaint)->refPathEffect().release());
}

sk_shader_t* sk_paint_get_shader(sk_paint_t* cpaint) {
    return reinterpret_cast<sk_shader_t*>(reinterpret_cast<SkPaint*>(cpaint)->refShader().release());
}

sk_stroke_cap_t sk_paint_get_stroke_cap(const sk_paint_t* cpaint) {
    return (sk_stroke_cap_t)reinterpret_cast<const SkPaint*>(cpaint)->getStrokeCap();
}

sk_stroke_join_t sk_paint_get_stroke_join(const sk_paint_t* cpaint) {
    return (sk_stroke_join_t)reinterpret_cast<const SkPaint*>(cpaint)->getStrokeJoin();
}

float sk_paint_get_stroke_miter(const sk_paint_t* cpaint) {
    return reinterpret_cast<const SkPaint*>(cpaint)->getStrokeMiter();
}

float sk_paint_get_stroke_width(const sk_paint_t* cpaint) {
    return reinterpret_cast<const SkPaint*>(cpaint)->getStrokeWidth();
}

sk_paint_style_t sk_paint_get_style(const sk_paint_t* cpaint) {
    return (sk_paint_style_t)reinterpret_cast<const SkPaint*>(cpaint)->getStyle();
}

bool sk_paint_is_antialias(const sk_paint_t* cpaint) {
    return reinterpret_cast<const SkPaint*>(cpaint)->isAntiAlias();
}

bool sk_paint_is_dither(const sk_paint_t* cpaint) {
    return reinterpret_cast<const SkPaint*>(cpaint)->isDither();
}

sk_paint_t* sk_paint_new(void) {
    return reinterpret_cast<sk_paint_t*>(new SkPaint());
}

void sk_paint_reset(sk_paint_t* cpaint) {
    reinterpret_cast<SkPaint*>(cpaint)->reset();
}

void sk_paint_set_antialias(sk_paint_t* cpaint, bool aa) {
    reinterpret_cast<SkPaint*>(cpaint)->setAntiAlias(aa);
}

void sk_paint_set_blend_mode(sk_paint_t* paint, sk_blend_mode_t mode) {
    reinterpret_cast<SkPaint*>(paint)->setBlendMode((SkBlendMode)mode);
}

void sk_paint_set_color(sk_paint_t* cpaint, sk_color_t c) {
    reinterpret_cast<SkPaint*>(cpaint)->setColor(c);
}

void sk_paint_set_colorfilter(sk_paint_t* cpaint, sk_color_filter_t* cfilter) {
    reinterpret_cast<SkPaint*>(cpaint)->setColorFilter(sk_ref_sp(reinterpret_cast<SkColorFilter*>(cfilter)));
}

void sk_paint_set_dither(sk_paint_t* cpaint, bool isdither) {
    reinterpret_cast<SkPaint*>(cpaint)->setDither(isdither);
}

void sk_paint_set_imagefilter(sk_paint_t* cpaint, sk_image_filter_t* cfilter) {
    reinterpret_cast<SkPaint*>(cpaint)->setImageFilter(sk_ref_sp(reinterpret_cast<SkImageFilter*>(cfilter)));
}

void sk_paint_set_maskfilter(sk_paint_t* cpaint, sk_mask_filter_t* cfilter) {
    reinterpret_cast<SkPaint*>(cpaint)->setMaskFilter(sk_ref_sp(reinterpret_cast<SkMaskFilter*>(cfilter)));
}

void sk_paint_set_path_effect(sk_paint_t* cpaint, sk_path_effect_t* effect) {
    reinterpret_cast<SkPaint*>(cpaint)->setPathEffect(sk_ref_sp(reinterpret_cast<SkPathEffect*>(effect)));
}

void sk_paint_set_shader(sk_paint_t* cpaint, sk_shader_t* cshader) {
    reinterpret_cast<SkPaint*>(cpaint)->setShader(sk_ref_sp(reinterpret_cast<SkShader*>(cshader)));
}

void sk_paint_set_stroke_cap(sk_paint_t* cpaint, sk_stroke_cap_t ccap) {
    reinterpret_cast<SkPaint*>(cpaint)->setStrokeCap((SkPaint::Cap)ccap);
}

void sk_paint_set_stroke_join(sk_paint_t* cpaint, sk_stroke_join_t cjoin) {
    reinterpret_cast<SkPaint*>(cpaint)->setStrokeJoin((SkPaint::Join)cjoin);
}

void sk_paint_set_stroke_miter(sk_paint_t* cpaint, float miter) {
    reinterpret_cast<SkPaint*>(cpaint)->setStrokeMiter(miter);
}

void sk_paint_set_stroke_width(sk_paint_t* cpaint, float width) {
    reinterpret_cast<SkPaint*>(cpaint)->setStrokeWidth(width);
}

void sk_paint_set_style(sk_paint_t* cpaint, sk_paint_style_t style) {
    reinterpret_cast<SkPaint*>(cpaint)->setStyle((SkPaint::Style)style);
}

// ===== Functions from include/core/SkPath.h =====
void sk_path_add_circle(sk_path_t* cpath, float x, float y, float radius, sk_path_direction_t dir) {
    reinterpret_cast<SkPath*>(cpath)->addCircle(x, y, radius, (SkPathDirection)dir);
}

void sk_path_add_oval(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir) {
    reinterpret_cast<SkPath*>(cpath)->addOval(*reinterpret_cast<const SkRect*>(crect), (SkPathDirection)cdir);
}

void sk_path_add_path(sk_path_t* cpath, sk_path_t* other, sk_path_add_mode_t add_mode) {
    reinterpret_cast<SkPath*>(cpath)->addPath(reinterpret_cast<SkPath&>(*other), (SkPath::AddPathMode) add_mode);
}

void sk_path_add_path_matrix(sk_path_t* cpath, sk_path_t* other, sk_matrix_t *matrix, sk_path_add_mode_t add_mode) {
    reinterpret_cast<SkPath*>(cpath)->addPath(reinterpret_cast<SkPath&>(*other), AsMatrix(matrix), (SkPath::AddPathMode) add_mode);
}

void sk_path_add_path_offset(sk_path_t* cpath, sk_path_t* other, float dx, float dy, sk_path_add_mode_t add_mode) {
    reinterpret_cast<SkPath*>(cpath)->addPath(reinterpret_cast<SkPath&>(*other), dx, dy, (SkPath::AddPathMode) add_mode);
}

void sk_path_add_path_reverse(sk_path_t* cpath, sk_path_t* other) {
    reinterpret_cast<SkPath*>(cpath)->reverseAddPath(reinterpret_cast<SkPath&>(*other));
}

void sk_path_add_poly(sk_path_t* cpath, const sk_point_t* points, int count, bool close) {
    reinterpret_cast<SkPath*>(cpath)->addPoly(reinterpret_cast<const SkPoint*>(points), count, close);
}

void sk_path_add_rect(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir) {
    reinterpret_cast<SkPath*>(cpath)->addRect(*reinterpret_cast<const SkRect*>(crect), (SkPathDirection)cdir);
}

void sk_path_add_rounded_rect(sk_path_t* cpath, const sk_rect_t* crect, float rx, float ry, sk_path_direction_t cdir) {
    reinterpret_cast<SkPath*>(cpath)->addRoundRect(*reinterpret_cast<const SkRect*>(crect), rx, ry, (SkPathDirection)cdir);
}

void sk_path_arc_to(sk_path_t* cpath, float rx, float ry, float xAxisRotate, sk_path_arc_size_t largeArc, sk_path_direction_t sweep, float x, float y) {
    reinterpret_cast<SkPath*>(cpath)->arcTo(rx, ry, xAxisRotate, (SkPath::ArcSize)largeArc, (SkPathDirection)sweep, x, y);
}

void sk_path_arc_to_with_oval(sk_path_t* cpath, const sk_rect_t* oval, float startAngle, float sweepAngle, bool forceMoveTo) {
    reinterpret_cast<SkPath*>(cpath)->arcTo(*reinterpret_cast<const SkRect*>(oval), startAngle, sweepAngle, forceMoveTo);
}

void sk_path_arc_to_with_points(sk_path_t* cpath, float x1, float y1, float x2, float y2, float radius) {
    reinterpret_cast<SkPath*>(cpath)->arcTo(x1, y1, x2, y2, radius);
}

sk_path_t* sk_path_clone(const sk_path_t* cpath) {
    return reinterpret_cast<sk_path_t*>(new SkPath(*reinterpret_cast<const SkPath*>(cpath)));
}

void sk_path_close(sk_path_t* cpath) {
    reinterpret_cast<SkPath*>(cpath)->close();
}

void sk_path_compute_tight_bounds(const sk_path_t* cpath, sk_rect_t* crect) {
    SkRect r = reinterpret_cast<const SkPath*>(cpath)->computeTightBounds();
    *crect = reinterpret_cast<sk_rect_t&>(r);
}

void sk_path_conic_to(sk_path_t* cpath, float x0, float y0, float x1, float y1, float w) {
    reinterpret_cast<SkPath*>(cpath)->conicTo(x0, y0, x1, y1, w);
}

bool sk_path_contains (const sk_path_t* cpath, float x, float y) {
    return reinterpret_cast<const SkPath*>(cpath)->contains(x, y);
}

int sk_path_count_points(const sk_path_t* cpath) {
    return reinterpret_cast<const SkPath*>(cpath)->countPoints();
}

void sk_path_cubic_to(sk_path_t* cpath, float x0, float y0, float x1, float y1, float x2, float y2) {
    reinterpret_cast<SkPath*>(cpath)->cubicTo(x0, y0, x1, y1, x2, y2);
}

void sk_path_delete(sk_path_t* cpath) {
    delete reinterpret_cast<SkPath*>(cpath);
}

void sk_path_get_bounds(const sk_path_t* cpath, sk_rect_t* crect) {
    SkRect r = reinterpret_cast<const SkPath*>(cpath)->getBounds();
    *crect = reinterpret_cast<sk_rect_t&>(r);
}

int sk_path_get_points(const sk_path_t* cpath, sk_point_t* points, int max) {
    return reinterpret_cast<const SkPath*>(cpath)->getPoints(reinterpret_cast<SkPoint*>(points), max);
}

sk_path_fill_type_t sk_path_get_filltype(sk_path_t *cpath) {
    return (sk_path_fill_type_t)reinterpret_cast<SkPath*>(cpath)->getFillType();
}

bool sk_path_get_last_point(const sk_path_t* cpath, sk_point_t* point) {
    return reinterpret_cast<const SkPath*>(cpath)->getLastPt(reinterpret_cast<SkPoint*>(point));
}

void sk_path_line_to(sk_path_t* cpath, float x, float y) {
    reinterpret_cast<SkPath*>(cpath)->lineTo(x, y);
}

void sk_path_move_to(sk_path_t* cpath, float x, float y) {
    reinterpret_cast<SkPath*>(cpath)->moveTo(x, y);
}

sk_path_t* sk_path_new(void) {
    return reinterpret_cast<sk_path_t*>(new SkPath());
}

bool sk_path_parse_svg_string(sk_path_t* cpath, const char* str) {
    return SkParsePath::FromSVGString(str, reinterpret_cast<SkPath*>(cpath));
}

void sk_path_quad_to(sk_path_t* cpath, float x0, float y0, float x1, float y1) {
    reinterpret_cast<SkPath*>(cpath)->quadTo(x0, y0, x1, y1);
}

void sk_path_rarc_to(sk_path_t* cpath, float rx, float ry, float xAxisRotate, sk_path_arc_size_t largeArc, sk_path_direction_t sweep, float x, float y) {
    reinterpret_cast<SkPath*>(cpath)->rArcTo(rx, ry, xAxisRotate, (SkPath::ArcSize)largeArc, (SkPathDirection)sweep, x, y);
}

void sk_path_rconic_to(sk_path_t* cpath, float dx0, float dy0, float dx1, float dy1, float w) {
    reinterpret_cast<SkPath*>(cpath)->rConicTo(dx0, dy0, dx1, dy1, w);
}

void sk_path_rcubic_to(sk_path_t* cpath, float dx0, float dy0, float dx1, float dy1, float dx2, float dy2) {
    reinterpret_cast<SkPath*>(cpath)->rCubicTo(dx0, dy0, dx1, dy1, dx2, dy2);
}

void sk_path_reset(sk_path_t* cpath) {
    reinterpret_cast<SkPath*>(cpath)->reset ();
}

void sk_path_rewind(sk_path_t* cpath) {
    reinterpret_cast<SkPath*>(cpath)->rewind ();
}

void sk_path_rline_to(sk_path_t* cpath, float dx, float dy) {
    reinterpret_cast<SkPath*>(cpath)->rLineTo(dx, dy);
}

void sk_path_rmove_to(sk_path_t* cpath, float dx, float dy) {
    reinterpret_cast<SkPath*>(cpath)->rMoveTo(dx, dy);
}

void sk_path_set_filltype(sk_path_t* cpath, sk_path_fill_type_t cfilltype) {
    reinterpret_cast<SkPath*>(cpath)->setFillType((SkPathFillType)cfilltype);
}

sk_string_t* sk_path_to_svg_string(const sk_path_t* cpath, bool absolute) {
    return reinterpret_cast<sk_string_t*>(new SkString(SkParsePath::ToSVGString(*reinterpret_cast<const SkPath*>(cpath), absolute ? SkParsePath::PathEncoding::Absolute : SkParsePath::PathEncoding::Relative)));
}

void sk_path_transform(sk_path_t* cpath, const sk_matrix_t* cmatrix) {
    reinterpret_cast<SkPath*>(cpath)->transform(AsMatrix(cmatrix));
}

void sk_path_transform_to_dest(const sk_path_t* cpath, const sk_matrix_t* cmatrix, sk_path_t* destination) {
    reinterpret_cast<const SkPath*>(cpath)->transform(AsMatrix(cmatrix), reinterpret_cast<SkPath*>(destination));
}

// ===== Functions from include/core/SkPathEffect.h =====
sk_path_effect_t* sk_path_effect_create_1d_path(const sk_path_t* path, float advance, float phase, sk_path_effect_1d_style_t style) {
    return reinterpret_cast<sk_path_effect_t*>(SkPath1DPathEffect::Make(*reinterpret_cast<const SkPath*>(path), advance, phase, (SkPath1DPathEffect::Style)style).release());
}

sk_path_effect_t* sk_path_effect_create_2d_line(float width, const sk_matrix_t* cmatrix) {
    return reinterpret_cast<sk_path_effect_t*>(SkLine2DPathEffect::Make(width, AsMatrix(cmatrix)).release());
}

sk_path_effect_t* sk_path_effect_create_2d_path(const sk_matrix_t* cmatrix, const sk_path_t* path) {
    return reinterpret_cast<sk_path_effect_t*>(SkPath2DPathEffect::Make(AsMatrix(cmatrix), *reinterpret_cast<const SkPath*>(path)).release());
}

sk_path_effect_t* sk_path_effect_create_compose(sk_path_effect_t* outer, sk_path_effect_t* inner) {
    return reinterpret_cast<sk_path_effect_t*>(SkPathEffect::MakeCompose(sk_ref_sp(reinterpret_cast<SkPathEffect*>(outer)), sk_ref_sp(reinterpret_cast<SkPathEffect*>(inner))).release());
}

sk_path_effect_t* sk_path_effect_create_corner(float radius) {
    return reinterpret_cast<sk_path_effect_t*>(SkCornerPathEffect::Make(radius).release());
}

sk_path_effect_t* sk_path_effect_create_dash(const float intervals[], int count, float phase) {
    return reinterpret_cast<sk_path_effect_t*>(SkDashPathEffect::Make(intervals, count, phase).release());
}

sk_path_effect_t* sk_path_effect_create_discrete(float segLength, float deviation, uint32_t seedAssist) {
    return reinterpret_cast<sk_path_effect_t*>(SkDiscretePathEffect::Make(segLength, deviation, seedAssist).release());
}

sk_path_effect_t* sk_path_effect_create_sum(sk_path_effect_t* first, sk_path_effect_t* second) {
    return reinterpret_cast<sk_path_effect_t*>(SkPathEffect::MakeSum(sk_ref_sp(reinterpret_cast<SkPathEffect*>(first)), sk_ref_sp(reinterpret_cast<SkPathEffect*>(second))).release());
}

sk_path_effect_t* sk_path_effect_create_trim(float start, float stop, sk_path_effect_trim_mode_t mode) {
    return reinterpret_cast<sk_path_effect_t*>(SkTrimPathEffect::Make(start, stop, (SkTrimPathEffect::Mode)mode).release());
}

void sk_path_effect_unref(sk_path_effect_t* effect) {
    SkSafeUnref(reinterpret_cast<SkPathEffect*>(effect));
}

// ===== Functions from include/pathops/SkPathOps.h =====
bool sk_path_op(const sk_path_t* path, const sk_path_t* other, sk_path_op_t op, sk_path_t *result) {
    return Op(*reinterpret_cast<const SkPath*>(path), *reinterpret_cast<const SkPath*>(other), (SkPathOp)op, reinterpret_cast<SkPath*>(result));
}

bool sk_path_simplify(const sk_path_t* path, sk_path_t *result) {
    return Simplify(*reinterpret_cast<const SkPath*>(path), reinterpret_cast<SkPath*>(result));
}

void sk_opbuilder_add(sk_op_builder_t* builder, const sk_path_t* path, sk_path_op_t op) {
    reinterpret_cast<SkOpBuilder*>(builder)->add(*reinterpret_cast<const SkPath*>(path), (SkPathOp)op);
}

void sk_opbuilder_destroy(sk_op_builder_t* builder) {
    delete reinterpret_cast<SkOpBuilder*>(builder);
}

sk_op_builder_t* sk_opbuilder_new(void) {
    return reinterpret_cast<sk_op_builder_t*>(new SkOpBuilder());
}

bool sk_opbuilder_resolve(sk_op_builder_t* builder, sk_path_t* result) {
    return reinterpret_cast<SkOpBuilder*>(builder)->resolve(reinterpret_cast<SkPath*>(result));
}

// ===== Functions from include/core/SkShader.h =====
sk_shader_t* sk_shader_new_blend(sk_blend_mode_t mode, const sk_shader_t* dst, const sk_shader_t* src) {
    return reinterpret_cast<sk_shader_t*>(SkShaders::Blend((SkBlendMode)mode, sk_ref_sp(reinterpret_cast<const SkShader*>(dst)), sk_ref_sp(reinterpret_cast<const SkShader*>(src))).release());
}

sk_shader_t* sk_shader_new_color(sk_color_t color) {
    return reinterpret_cast<sk_shader_t*>(SkShaders::Color(color).release());
}

sk_shader_t* sk_shader_new_linear_gradient(const sk_point_t points[2], const sk_color_t colors[], const float colorPos[], int colorCount, sk_tile_mode_t tileMode, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix) {
        m = AsMatrix(localMatrix);
    }
    return reinterpret_cast<sk_shader_t*>(SkGradientShader::MakeLinear(reinterpret_cast<const SkPoint*>(points), colors, colorPos, colorCount, (SkTileMode)tileMode, 0, localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_perlin_noise_fractal_noise(float baseFrequencyX, float baseFrequencyY, int numOctaves, float seed, const sk_isize_t* tileSize) {
    return reinterpret_cast<sk_shader_t*>(SkShaders::MakeFractalNoise(baseFrequencyX, baseFrequencyY, numOctaves, seed, reinterpret_cast<const SkISize*>(tileSize)).release());
}

sk_shader_t* sk_shader_new_perlin_noise_turbulence(float baseFrequencyX, float baseFrequencyY, int numOctaves, float seed, const sk_isize_t* tileSize) {
    return reinterpret_cast<sk_shader_t*>(SkShaders::MakeTurbulence(baseFrequencyX, baseFrequencyY,  numOctaves,  seed,  reinterpret_cast<const SkISize*>(tileSize)).release());
}

sk_shader_t* sk_shader_new_radial_gradient(const sk_point_t* center, float radius, const sk_color_t colors[], const float colorPos[], int colorCount, sk_tile_mode_t tileMode, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix) {
        m = AsMatrix(localMatrix);
    }
    return reinterpret_cast<sk_shader_t*>(SkGradientShader::MakeRadial(*reinterpret_cast<const SkPoint*>(center), (SkScalar)radius, colors, colorPos, colorCount, (SkTileMode)tileMode, 0, localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_sweep_gradient(const sk_point_t* center, const sk_color_t colors[], const float colorPos[], int colorCount, sk_tile_mode_t tileMode, float startAngle, float endAngle, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix) {
        m = AsMatrix(localMatrix);
    }
    return reinterpret_cast<sk_shader_t*>(SkGradientShader::MakeSweep(center->x, center->y, colors, colorPos, colorCount, (SkTileMode)tileMode, startAngle, endAngle, 0, localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_two_point_conical_gradient(const sk_point_t* start, float startRadius, const sk_point_t* end, float endRadius, const sk_color_t colors[], const float colorPos[], int colorCount, sk_tile_mode_t tileMode, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix) {
        m = AsMatrix(localMatrix);
    }
    return reinterpret_cast<sk_shader_t*>(SkGradientShader::MakeTwoPointConical(*reinterpret_cast<const SkPoint*>(start), startRadius, *reinterpret_cast<const SkPoint*>(end), endRadius, colors, colorPos, colorCount, (SkTileMode)tileMode, 0, localMatrix ? &m : nullptr).release());
}

void sk_shader_unref(sk_shader_t* shader) {
    SkSafeUnref(reinterpret_cast<SkShader*>(shader));
}

sk_shader_t* sk_shader_with_color_filter(const sk_shader_t* shader, const sk_color_filter_t* filter) {
    return reinterpret_cast<sk_shader_t*>(reinterpret_cast<const SkShader*>(shader)->makeWithColorFilter(sk_ref_sp(reinterpret_cast<const SkColorFilter*>(filter))).release());
}

sk_shader_t* sk_shader_with_local_matrix(const sk_shader_t* shader, const sk_matrix_t* localMatrix) {
    return reinterpret_cast<sk_shader_t*>(reinterpret_cast<const SkShader*>(shader)->makeWithLocalMatrix(AsMatrix(localMatrix)).release());
}

// ===== Functions from include/core/SkString.h =====
sk_string_t* sk_string_new(const char* text, size_t len) {
	return reinterpret_cast<sk_string_t*>(new SkString(text, len));
}

sk_string_t* sk_string_new_empty(void) {
    return reinterpret_cast<sk_string_t*>(new SkString());
}

void sk_string_delete(const sk_string_t* cstring) {
    delete reinterpret_cast<const SkString*>(cstring);
}

const char* sk_string_get_c_str(const sk_string_t* cstring) {
    return reinterpret_cast<const SkString*>(cstring)->c_str();
}

size_t sk_string_get_size(const sk_string_t* cstring) {
    return reinterpret_cast<const SkString*>(cstring)->size();
}

// ===== Functions from include/core/SkSurface.h =====
sk_surface_t* sk_surface_make_raster_direct(const sk_image_info_t *imageInfo, void *pixels, size_t rowBytes, sk_surface_props_t* surfaceProps) {
	return reinterpret_cast<sk_surface_t*>(SkSurfaces::WrapPixels(*reinterpret_cast<const SkImageInfo *>(imageInfo), pixels, rowBytes, reinterpret_cast<const SkSurfaceProps*>(surfaceProps)).release());
}

sk_surface_t* sk_surface_make_raster_n32_premul(const sk_image_info_t *imageInfo, sk_surface_props_t* surfaceProps) {
	return reinterpret_cast<sk_surface_t*>(SkSurfaces::Raster(*reinterpret_cast<const SkImageInfo *>(imageInfo), reinterpret_cast<const SkSurfaceProps*>(surfaceProps)).release());
}

sk_surface_t* sk_surface_make_surface(sk_surface_t *surface, int width, int height)
{
    return reinterpret_cast<sk_surface_t *>(reinterpret_cast<SkSurface *>(surface)->makeSurface(width, height).release());
}

sk_canvas_t* sk_surface_get_canvas(sk_surface_t* surface) {
    return reinterpret_cast<sk_canvas_t*>(reinterpret_cast<SkSurface*>(surface)->getCanvas());
}

sk_surface_t* sk_surface_new_backend_render_target(gr_direct_context_t* context, const gr_backendrendertarget_t* target, gr_surface_origin_t origin, sk_color_type_t colorType, sk_color_space_t* colorspace, const sk_surface_props_t* props) {
    return reinterpret_cast<sk_surface_t*>(SkSurfaces::WrapBackendRenderTarget(reinterpret_cast<GrDirectContext*>(context),
        *reinterpret_cast<const GrBackendRenderTarget*>(target), (GrSurfaceOrigin)origin, (SkColorType)colorType,
        sk_ref_sp(reinterpret_cast<SkColorSpace*>(colorspace)), reinterpret_cast<const SkSurfaceProps*>(props)).release());
}

sk_image_t* sk_surface_make_image_snapshot(sk_surface_t* surface) {
	return reinterpret_cast<sk_image_t*>(reinterpret_cast<SkSurface*>(surface)->makeImageSnapshot().release());
}

void sk_surface_unref(sk_surface_t* surface) {
    SkSafeUnref(reinterpret_cast<SkSurface*>(surface));
}

// ===== Functions from include/core/SkSurfaceProps.h =====
sk_surface_props_t* sk_surfaceprops_new(uint32_t flags, sk_pixel_geometry_t geometry) {
    return reinterpret_cast<sk_surface_props_t*>(new SkSurfaceProps(flags, (SkPixelGeometry)geometry));
}

void sk_surfaceprops_delete(sk_surface_props_t *surface_props)
{
    delete reinterpret_cast<SkSurfaceProps *>(surface_props);
}


// ===== Functions from include/core/SkTextBlob.h =====
const sk_text_blob_builder_run_buffer_t* sk_textblob_builder_alloc_run(sk_text_blob_builder_t* builder, const sk_font_t* font, int count, float x, float y, const sk_rect_t* bounds) {
    return &reinterpret_cast<const sk_text_blob_builder_run_buffer_t&>(reinterpret_cast<SkTextBlobBuilder*>(builder)->allocRun(reinterpret_cast<const SkFont&>(*font), count, x, y, reinterpret_cast<const SkRect*>(bounds)));
}

const sk_text_blob_builder_run_buffer_t* sk_textblob_builder_alloc_run_pos(sk_text_blob_builder_t* builder, const sk_font_t* font, int count, const sk_rect_t* bounds) {
    return &reinterpret_cast<const sk_text_blob_builder_run_buffer_t&>(reinterpret_cast<SkTextBlobBuilder*>(builder)->allocRunPos(reinterpret_cast<const SkFont&>(*font), count, reinterpret_cast<const SkRect*>(bounds)));
}

const sk_text_blob_builder_run_buffer_t* sk_textblob_builder_alloc_run_pos_h(sk_text_blob_builder_t* builder, const sk_font_t* font, int count, float y, const sk_rect_t* bounds) {
    return &reinterpret_cast<const sk_text_blob_builder_run_buffer_t&>(reinterpret_cast<SkTextBlobBuilder*>(builder)->allocRunPosH(reinterpret_cast<const SkFont&>(*font), count, y, reinterpret_cast<const SkRect*>(bounds)));
}

void sk_textblob_builder_delete(sk_text_blob_builder_t* builder) {
    delete reinterpret_cast<SkTextBlobBuilder*>(builder);
}

sk_text_blob_t* sk_textblob_builder_make(sk_text_blob_builder_t* builder) {
    return reinterpret_cast<sk_text_blob_t*>(reinterpret_cast<SkTextBlobBuilder*>(builder)->make().release());
}

sk_text_blob_builder_t* sk_textblob_builder_new(void) {
    return reinterpret_cast<sk_text_blob_builder_t*>(new SkTextBlobBuilder());
}

void sk_textblob_get_bounds(const sk_text_blob_t* blob, sk_rect_t* bounds) {
    *bounds = reinterpret_cast<const sk_rect_t&>(reinterpret_cast<const SkTextBlob*>(blob)->bounds());
}

int sk_textblob_get_intercepts(const sk_text_blob_t* blob, const float bounds[2], float intervals[], const sk_paint_t* paint) {
    return reinterpret_cast<const SkTextBlob*>(blob)->getIntercepts(bounds, intervals, reinterpret_cast<const SkPaint*>(paint));
}

sk_text_blob_t* sk_textblob_make_from_text(const void* text, size_t byteLength, const sk_font_t* font, sk_text_encoding_t encoding) {
    return reinterpret_cast<sk_text_blob_t*>(SkTextBlob::MakeFromText(text, byteLength, reinterpret_cast<const SkFont&>(*font), (SkTextEncoding)encoding).release());
}

void sk_textblob_unref(const sk_text_blob_t* blob) {
    SkSafeUnref(reinterpret_cast<const SkTextBlob*>(blob));
}

// ===== Functions from include/core/SkTypeface.h =====
sk_string_t* sk_typeface_get_family_name(const sk_typeface_t* typeface) {
    SkString* family_name = new SkString();
    reinterpret_cast<const SkTypeface*>(typeface)->getFamilyName(family_name);
    return reinterpret_cast<sk_string_t*>(family_name);
}

sk_font_style_t* sk_typeface_get_fontstyle(const sk_typeface_t* typeface) {
    SkFontStyle fs = reinterpret_cast<const SkTypeface*>(typeface)->fontStyle();
    return reinterpret_cast<sk_font_style_t*>(new SkFontStyle(fs.weight(), fs.width(), fs.slant()));
}

int sk_typeface_get_units_per_em(const sk_typeface_t* typeface) {
    return reinterpret_cast<const SkTypeface*>(typeface)->getUnitsPerEm();
}

bool sk_typeface_is_fixed_pitch(const sk_typeface_t* typeface) {
    return reinterpret_cast<const SkTypeface*>(typeface)->isFixedPitch();
}

void sk_typeface_unref(sk_typeface_t* typeface) {
    SkSafeUnref(reinterpret_cast<SkTypeface*>(typeface));
}

// ===== Functions from include/core/SkStream.h =====

sk_dynamic_memory_wstream_t* sk_dynamic_memory_wstream_new(void) {
	return reinterpret_cast<sk_dynamic_memory_wstream_t*>(new SkDynamicMemoryWStream());
}

sk_wstream_t* sk_dynamic_memory_wstream_as_wstream(sk_dynamic_memory_wstream_t* stream) {
	return reinterpret_cast<sk_wstream_t*>(stream);
}

bool sk_dynamic_memory_wstream_write(sk_dynamic_memory_wstream_t* stream, const void *buffer, size_t size) {
	return reinterpret_cast<SkDynamicMemoryWStream*>(stream)->write(buffer, size);
}

size_t sk_dynamic_memory_wstream_bytes_written(sk_dynamic_memory_wstream_t* stream) {
	return reinterpret_cast<SkDynamicMemoryWStream*>(stream)->bytesWritten();
}

size_t sk_dynamic_memory_wstream_read(sk_dynamic_memory_wstream_t* stream, void *buffer, size_t offset, size_t size) {
	return reinterpret_cast<SkDynamicMemoryWStream*>(stream)->read(buffer, offset, size);
}

void sk_dynamic_memory_wstream_delete(sk_dynamic_memory_wstream_t* stream) {
	delete reinterpret_cast<SkDynamicMemoryWStream*>(stream);
}

sk_file_wstream_t* sk_file_wstream_new(const char* path) {
	return reinterpret_cast<sk_file_wstream_t*>(new SkFILEWStream(path));
}

sk_wstream_t* sk_file_wstream_as_wstream(sk_file_wstream_t* stream) {
	return reinterpret_cast<sk_wstream_t*>(stream);
}

bool sk_file_wstream_write(sk_file_wstream_t* stream, const void *buffer, size_t size) {
	return reinterpret_cast<SkFILEWStream*>(stream)->write(buffer, size);
}

size_t sk_file_wstream_bytes_written(sk_file_wstream_t* stream) {
	return reinterpret_cast<SkFILEWStream*>(stream)->bytesWritten();
}

void sk_file_wstream_flush(sk_file_wstream_t* stream) {
	reinterpret_cast<SkFILEWStream*>(stream)->flush();
}

void sk_file_wstream_delete(sk_file_wstream_t* stream) {
	delete reinterpret_cast<SkFILEWStream*>(stream);
}

// ===== Functions from include/core/SkDocument.h =====
sk_canvas_t* sk_document_begin_page(sk_document_t* doc, float width, float height) {
	return reinterpret_cast<sk_canvas_t*>(reinterpret_cast<SkDocument*>(doc)->beginPage(width, height));
}

void sk_document_end_page(sk_document_t* doc) {
	reinterpret_cast<SkDocument*>(doc)->endPage();
}

void sk_document_close(sk_document_t* doc) {
	reinterpret_cast<SkDocument*>(doc)->close();
}

void sk_document_abort(sk_document_t* doc) {
	reinterpret_cast<SkDocument*>(doc)->abort();
}

// ===== Functions from include/docs/SkPDFDocument.h =====

static void sk_convertDateTime(SkPDF::DateTime* to, sk_date_time_t* from) {
	to->fTimeZoneMinutes = from->timeZoneMinutes;
	to->fYear = from->year;
	to->fMonth = from->month;
	to->fDayOfWeek = from->dayOfWeek;
	to->fDay = from->day;
	to->fHour = from->hour;
	to->fMinute = from->minute;
	to->fSecond = from->second;
}

sk_document_t* sk_document_make_pdf(sk_wstream_t* stream, sk_metadata_t* metadata) {
	SkPDF::Metadata md;
	if (metadata->title) {
		md.fTitle = metadata->title;
	}
	if (metadata->author) {
		md.fAuthor = metadata->author;
	}
	if (metadata->subject) {
		md.fSubject = metadata->subject;
	}
	if (metadata->keywords) {
		md.fKeywords = metadata->keywords;
	}
	if (metadata->creator) {
		md.fCreator = metadata->creator;
	}
	if (metadata->producer) {
		md.fProducer = metadata->producer;
	}
	sk_convertDateTime(&md.fCreation, &metadata->creation);
	sk_convertDateTime(&md.fModified, &metadata->modified);
	md.fRasterDPI = metadata->rasterDPI;
	md.fEncodingQuality = metadata->encodingQuality;
	return reinterpret_cast<sk_document_t*>(new SkPDFDocument(reinterpret_cast<SkWStream*>(stream), md));
}

// ===== Functions from include/codec/SkCodec.h =====

void register_image_codecs() {
    SkCodecs::Register(SkPngDecoder::Decoder());
    SkCodecs::Register(SkJpegDecoder::Decoder());
    SkCodecs::Register(SkWebpDecoder::Decoder());
    SkCodecs::Register(SkGifDecoder::Decoder());
    SkCodecs::Register(SkIcoDecoder::Decoder());
    SkCodecs::Register(SkBmpDecoder::Decoder());
    SkCodecs::Register(SkWbmpDecoder::Decoder());
}
