/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.
#include "FontEffectShadowInstancer.h"
#include "FontEffectShadow.h"
#include "precompiled.h"

namespace Rocket {
namespace Core {

FontEffectShadowInstancer::FontEffectShadowInstancer(Context* _context)
  : FontEffectInstancer(_context) {
  RegisterProperty("offset-x", "0", true).AddParser("number");
  RegisterProperty("offset-y", "0", true).AddParser("number");
  RegisterShorthand("offset", "offset-x, offset-y");
}

FontEffectShadowInstancer::~FontEffectShadowInstancer() {}

// Instances an outline font effect.
FontEffect* FontEffectShadowInstancer::InstanceFontEffect(Context* _context,
    const String& ROCKET_UNUSED_PARAMETER(name),
    const PropertyDictionary& _properties) {
  Vector2i offset;
  offset.x =
      Math::RealToInteger(_properties.GetProperty("offset-x")->Get<float>());
  offset.y =
      Math::RealToInteger(_properties.GetProperty("offset-y")->Get<float>());

  FontEffectShadow* font_effect = new FontEffectShadow(_context);
  if (font_effect->Initialise(offset))
    return font_effect;

  font_effect->RemoveReference();
  ReleaseFontEffect(font_effect);
  return NULL;
}

// Releases the given font effect.
void FontEffectShadowInstancer::ReleaseFontEffect(FontEffect* font_effect) {
  delete font_effect;
}

// Releases the instancer.
void FontEffectShadowInstancer::Release() {
  delete this;
}
}  // namespace Core
}  // namespace Rocket
