#include <cstring>
#include "doris_rinex_details.hpp"

int dso::doris_rnx::Beacon::set_from_rinex_line(const char *line) noexcept {
  if (line[0] != 'D') {
    fprintf(stderr,
            "[ERROR] Rinex 'STATION REFERENCE' fields should start with a 'D'! "
            "(traceback: %s)\n",
            __func__);
    return 1;
  }

  /* copy line as-is */
  std::memcpy(mpool, line, 52 * sizeof(char));

  /* correct the last char (i.e. beacon type) e.g. from '0' to 0 */
  mpool[m_type_at] = mpool[m_type_at] - '0';

  return 0;
}
