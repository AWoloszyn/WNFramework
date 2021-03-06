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
#include "XMLNodeHandlerBody.h"
#include "../../Include/Rocket/Core.h"
#include "XMLParseTools.h"
#include "precompiled.h"

namespace Rocket {
namespace Core {

XMLNodeHandlerBody::XMLNodeHandlerBody(Context* _context)
  : m_context(_context) {}

XMLNodeHandlerBody::~XMLNodeHandlerBody() {}

Element* XMLNodeHandlerBody::ElementStart(XMLParser* parser,
    const String& ROCKET_UNUSED_ASSERT_PARAMETER(name),
    const XMLAttributes& attributes) {
  ROCKET_UNUSED_ASSERT(name);
  ROCKET_ASSERT(name == "body");

  Element* element = parser->GetParseFrame()->element;

  // Check for and apply any template
  String template_name = attributes.Get<String>("template", "");
  if (!template_name.Empty()) {
    element = XMLParseTools::ParseTemplate(m_context, element, template_name);
  }

  // Apply any attributes to the document
  ElementDocument* document =
      parser->GetParseFrame()->element->GetOwnerDocument();
  if (document)
    document->SetAttributes(&attributes);

  // Tell the parser to use the element handler for all children
  parser->PushDefaultHandler();

  return element;
}

bool XMLNodeHandlerBody::ElementEnd(XMLParser* ROCKET_UNUSED_PARAMETER(parser),
    const String& ROCKET_UNUSED_PARAMETER(name)) {
  return true;
}

bool XMLNodeHandlerBody::ElementData(XMLParser* parser, const String& data) {
  return Factory::InstanceElementText(
      m_context, parser->GetParseFrame()->element, data);
}

void XMLNodeHandlerBody::Release() {
  delete this;
}
}  // namespace Core
}  // namespace Rocket
