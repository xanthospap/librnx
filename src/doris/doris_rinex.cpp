#include "doris_rinex.hpp"
#include <stdexcept>

/** The constructor will try to:
 *  1. open the input file
 *  2. parse the header
 *  If any of the above fails, then an std::runtime_error will be thrown.
 */
dso::DorisObsRinex::DorisObsRinex(const char *fn)
    : m_filename(fn), m_stream(fn, std::ios_base::in) {
  /* pre-allocate vectors */
  m_obs_codes.reserve(10);
  m_obs_scale_factors.reserve(10);
  m_stations.reserve(65);
  m_ref_stations.reserve(7);

  /* read the header .. */
  try {
    int status = read_header();
    if (status) {
      fprintf(
          stderr,
          "[ERROR] Failed reading RINEX header for %s (error=%d) (traceback: "
          "%s)\n",
          fn, status, __func__);
      throw std::runtime_error("[ERROR] Cannot read RINEX header");
    }
  } catch (std::exception &) {
    fprintf(stderr, "[ERROR] Failed creating DorisObsRinex instance\n");
  }
}

dso::DorisObsRinex::~DorisObsRinex() noexcept = default;

