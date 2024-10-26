#pragma once

#include <Adafruit_NeoPixel.h>
#include <PrimitiveExpeditor.h>

namespace uniot {
class Pixel {
 public:
  Pixel(uint16_t n, int16_t pin, neoPixelType type = NEO_RGB + NEO_KHZ800)
      : mPixels(n, pin, type) {}

  Object primitiveSet(Root root, VarObject env, VarObject list) {
    auto expeditor = PrimitiveExpeditor::describe("pixel_set", Lisp::Bool, 4, Lisp::Int, Lisp::Int, Lisp::Int, Lisp::Int)
                         .init(root, env, list);
    expeditor.assertDescribedArgs();
    auto pixel = expeditor.getArgInt(0);
    auto r = expeditor.getArgInt(1);
    auto g = expeditor.getArgInt(2);
    auto b = expeditor.getArgInt(3);

    if (pixel < 0 || pixel >= mPixels.numPixels()) {
      return expeditor.makeBool(false);
    }

    r = constrain(r, 0, 255);
    g = constrain(g, 0, 255);
    b = constrain(b, 0, 255);

    mPixels.setPixelColor(pixel, r, g, b);

    return expeditor.makeBool(true);
  }

  Object primitiveClear(Root root, VarObject env, VarObject list) {
    auto expeditor = PrimitiveExpeditor::describe("pixel_clear", Lisp::Bool, 0)
                         .init(root, env, list);
    expeditor.assertDescribedArgs();

    mPixels.clear();

    return expeditor.makeBool(true);
  }

  Object primitiveShow(Root root, VarObject env, VarObject list) {
    auto expeditor = PrimitiveExpeditor::describe("pixel_show", Lisp::Bool, 0)
                         .init(root, env, list);
    expeditor.assertDescribedArgs();

    mPixels.show();

    return expeditor.makeBool(true);
  }

 private:
  Adafruit_NeoPixel mPixels;
};

}  // namespace uniot