#include <stdexcept>

#include "doris_rinex.hpp"

dso::DorisObsRinex::iterator &dso::DorisObsRinex::iterator::operator++() {
  /* already at end */
  if (!m_rnx) return *this;

  /* try to get next block */
  int status = m_rnx->get_next_data_block(m_current);
  if (!status) return *this;

  /* eof/end ? */
  if (status < 0) {
    m_rnx = nullptr;
    return *this;
  }

  throw std::runtime_error(
      "[ERROR] Failed getting next data block from RINEX file " +
      m_rnx->m_filename + "\n");
}
