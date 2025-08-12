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

  /* now, we need to place some nulls to mark different strings */
  /* Example: 'D31  DIOB DIONYSOS                      12602S012  4'
   *          #0123456789012345678901234567890123456789012345678901'
   *                     10        20        30        40        50
   * where all whitespace character are stored as '\0'.
   */
  mpool[3] = '\0';
  mpool[9] = '\0';
  mpool[39] = '\0';
  mpool[49] = '\0';
  mpool[50] = '\0';

  return 0;
}
