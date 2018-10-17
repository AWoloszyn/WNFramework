// Copyright (c) 2018, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include "renderer/inc/render_data.h"
#include "WNScripting/inc/WNEngine.h"

using namespace wn::engine;
using namespace wn;

namespace wn {
namespace engine {
namespace renderer {
void render_data::register_scripting(scripting::engine* _engine) {
  _engine->export_script_type<render_data>();
}

void render_data::finalize_scripting(scripting::engine* _engine) {
  _engine->resolve_script_type<render_data>();
}

}  // namespace renderer
}  // namespace engine
}  // namespace wn