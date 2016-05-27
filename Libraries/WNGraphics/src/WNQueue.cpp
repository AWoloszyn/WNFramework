#include "WNGraphics/inc/Internal/WNConfig.h"
#include "WNGraphics/inc/WNDevice.h"
#include "WNGraphics/inc/WNQueue.h"

namespace wn {
namespace graphics {

#ifdef _WN_GRAPHICS_SINGLE_DEVICE_TYPE
queue::~queue() {
  if (is_valid()) {
    m_device->destroy_queue(this);
  }
}
#endif

}  // namespace graphics
}  // namespace wn