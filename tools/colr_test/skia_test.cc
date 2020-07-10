
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"

#include "include/ports/SkFontMgr_empty.h"

#include "tools/flags/CommandLineFlags.h"

#include <iterator>
#include <string>

static DEFINE_string(font, "skia_test/samples-colr_1.ttf", "Font to load.");
static DEFINE_string(text, "duck", "Text to render in font.");
static DEFINE_string(output, "colr_v1_glyph.png", "File to write.");

const uint16_t kTestGlyphIds[] = {13, 14};
// const char kTestFileName[] = "skia_test/noto-emoji-glyf_colr_1.ttf";
// const uint16_t kTestGlyphIds[] = {6,  10, 14, 17, 18, 26, 31, 32,
//                                   35, 37, 38, 41, 43, 53, 54};
const float kFontSize = 128;

int main(int argc, char** argv) {
  CommandLineFlags::Parse(argc, argv);

  SkImageInfo image_info =
      SkImageInfo::Make(2000, 200, SkColorType::kRGB_565_SkColorType,
                        SkAlphaType::kOpaque_SkAlphaType);
  sk_sp<SkSurface> surface(SkSurface::MakeRaster(image_info));
  if (!surface)
    return 1;
  SkCanvas* canvas = surface->getCanvas();
  canvas->clear(SK_ColorWHITE);
  SkPaint paint;

  auto font_mgr = sk_sp<SkFontMgr>(SkFontMgr_New_Custom_Empty());
  auto font_file_stream = SkStream::MakeFromFile(FLAGS_font[0]);
  sk_sp<SkTypeface> typeface(
      font_mgr->makeFromStream(std::move(font_file_stream)));

  SkASSERT(typeface);

  SkFont font(typeface);
  font.setSize(kFontSize);

  for (size_t i = 0; i < sizeof(kTestGlyphIds) / sizeof(uint16_t); ++i) {
    canvas->drawSimpleText(&kTestGlyphIds[i], 2, SkTextEncoding::kGlyphID,
                           10 + kFontSize * (i), kFontSize + 10, font, paint);
  }

  sk_sp<SkImage> image = surface->makeImageSnapshot();
  sk_sp<SkData> png = image->encodeToData(SkEncodedImageFormat::kPNG, 100);
  SkFILEWStream{FLAGS_output[0]}.write(png->data(), png->size());
}
