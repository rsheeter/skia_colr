
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

#include "hb-ot.h"

#include <iterator>
#include <string>

static DEFINE_string(font, "skia_test/samples-colr_1.ttf", "Font to load.");
static DEFINE_string(text, "simple_linearsimple_radial", "Text to render in font.");
static DEFINE_string(output, "colr_v1_glyph.png", "File to write.");
static DEFINE_bool(show_bbox, false, "Whether to draw bbox markers.");

const double kFontSizeScale = 64.0f;
const float kFontSize = 128;
const float kMargin = 32;

int main(int argc, char** argv) {
  CommandLineFlags::Parse(argc, argv);

  auto font_mgr = sk_sp<SkFontMgr>(SkFontMgr_New_Custom_Empty());

  // Shape & thus measure our text
  // Initially based on https://harfbuzz.github.io/ch03s03.html
  // until that turned out to be insufficient to compile.
  // Then based on https://github.com/aam/skiaex/blob/master/app/main.cpp,
  // which is referenced from Skia faq about how to get shaping.

  auto data = SkData::MakeFromFileName(FLAGS_font[0]);
  std::unique_ptr<SkStreamAsset> data_stream(new SkMemoryStream(data));
  sk_sp<SkTypeface> face(
      font_mgr->makeFromStream(std::move(data_stream)));
  SkASSERT(face);


  SkFont font(face);
  font.setSize(kFontSize);

  // get a blob of our font file
  auto destroy = [](void *d) { static_cast<SkData*>(d)->unref(); };
  const char* bytes = (const char*)data->data();
  unsigned int size = (unsigned int)data->size();
  hb_blob_t* hb_font_blob = hb_blob_create(bytes,
                                           size,
                                           HB_MEMORY_MODE_READONLY,
                                           data.release(),
                                           destroy);
  hb_blob_make_immutable(hb_font_blob);

  // We'll need an hb face & font to shape
  hb_face_t* hb_face = hb_face_create(hb_font_blob, /* index */ 0);
  hb_blob_destroy(hb_font_blob);

  hb_font_t *hb_font = hb_font_create(hb_face);
    hb_font_set_scale(hb_font,
        kFontSizeScale * kFontSize,
        kFontSizeScale * kFontSize);
    hb_ot_font_set_funcs(hb_font);

  // Let's all agree on upem
  hb_face_set_upem(hb_face, face->getUnitsPerEm());

  // Oh wise and powerful HarfBuzz, what glyphs and where?
  hb_buffer_t *hb_buffer = hb_buffer_create ();
  hb_buffer_add_utf8 (hb_buffer, FLAGS_text[0], -1, 0, -1);
  hb_buffer_guess_segment_properties (hb_buffer);

  hb_shape (hb_font, hb_buffer, NULL, 0);

  // Stand back, I'm thinking about drawing
  SkTextBlobBuilder textBlobBuilder;
  unsigned len = hb_buffer_get_length (hb_buffer);

  hb_glyph_info_t *info = hb_buffer_get_glyph_infos (hb_buffer, NULL);
  hb_glyph_position_t *pos = hb_buffer_get_glyph_positions (hb_buffer, NULL);
  auto runBuffer = textBlobBuilder.allocRunPos(font, len);

  double x = 0;
  double y = 0;
  for (unsigned int i = 0; i < len; i++)
  {
    runBuffer.glyphs[i] = info[i].codepoint;
    printf("gid %d (%.01f %.01f)\n", info[i].codepoint, x, y);
    reinterpret_cast<SkPoint*>(runBuffer.pos)[i] = SkPoint::Make(
      x + pos[i].x_offset / kFontSizeScale,
      y - pos[i].y_offset / kFontSizeScale);

    x += pos[i].x_advance / kFontSizeScale;
    y += pos[i].y_advance / kFontSizeScale;
  }

  auto textBlob = textBlobBuilder.make();

  // How much space y'all need?
  auto bbox = textBlob->bounds();
  x = 2 * kMargin + bbox.width();
  y = 2 * kMargin + bbox.height();

  // Let's paint something!
  printf("Making %.1f x %.1f canvas\n", x, y);
  SkImageInfo image_info =
      SkImageInfo::Make(x, y, SkColorType::kRGB_565_SkColorType,
                        SkAlphaType::kOpaque_SkAlphaType);
  sk_sp<SkSurface> surface(SkSurface::MakeRaster(image_info));
  if (!surface)
    return 1;
  SkCanvas* canvas = surface->getCanvas();
  canvas->clear(SK_ColorWHITE);

  SkPaint paint;

  if (FLAGS_show_bbox) {
    canvas->drawLine(kMargin, kMargin, x - kMargin, kMargin, paint);
    canvas->drawLine(kMargin, y - kMargin, x - kMargin, y - kMargin, paint);
  }

  canvas->drawTextBlob(textBlob, kMargin, y - kMargin, paint);

  sk_sp<SkImage> image = surface->makeImageSnapshot();
  sk_sp<SkData> png = image->encodeToData(SkEncodedImageFormat::kPNG, 100);
  SkFILEWStream{FLAGS_output[0]}.write(png->data(), png->size());

  // Cleanup
  hb_buffer_destroy (hb_buffer);
  hb_font_destroy(hb_font);
  hb_face_destroy(hb_face);
}
