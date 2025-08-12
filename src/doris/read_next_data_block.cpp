#include <charconv>
#include <cstring>
#include <exception>
#include <stdexcept>

#include "datetime/datetime_read.hpp"
#include "doris_rinex.hpp"

namespace {

const char *skipws(const char *line) noexcept {
  while (*line && *line == ' ') ++line;
  return line;
}

/*  Example line:
 *
 *  > 2020 01 01 01 41 53.279947800  0  4       -4.432841287 0
 *    +-------------------------+-----------+-----------+----+
 *    | Record identifier : '>' |  A1       | start:  0 | 1
 *    +-------------------------+-----------+-----------+----+
 *    | Epoch :                 |           |
 *    | - year (4 digits)       | 1X,I4     | start:  1 | 5
 *    | - month,day,hour,min    | 4(1X,I2.2)| start:  6 | 12
 *    | - sec                   | F13.9     | start: 18 | 13
 *    +-------------------------+-----------+-----------+----+
 *    |Epoch flag               | 2X,I1     | start: 31 | 3
 *    |   0: OK                             |           |
 *    |   1: power failure between          |           |
 *    |      previous and current epoch     |           |
 *    |  >1: Special event                  |           |
 *    +-------------------------+-----------+-----------+----+
 *    |Number of stations       | I3        | start: 34 | 3
 *    |observed in current epoch|           |           |
 *    +-------------------------+-----------+-----------+----+
 *    |(reserved)               | 6X        | start: 37 | 6
 *    +-------------------------+-----------+-----------+----+
 *    | Receiver clock offset   | F13.9     | start: 43 | 13
 *    | (seconds, optional)     |           |           |
 *    +-------------------------+-----------+-----------+----+
 *    | Receiver clock offset   |           | start: 56 | 3
 *    | flag,                   | 1X,I1,1X  |           |
 *    |  - 1 if extrapolated,   |           |           |
 *    |  - 0 otherwise          |           | Max length of line = 59 chars
 *    +-------------------------+-----------+------------------------------
 */
int resolve_block_epoch(const char *line,
                        dso::doris_rnx::RinexDataRecordHeader &hdr) noexcept {
  /* line must start with '>' character */
  if (*line != '>') return 1;

  int status = 0;
  int val;
  int sz = std::strlen(line);

  /* resolve date */
  try {
    hdr.m_epoch =
        dso::from_char<dso::YMDFormat::YYYYMMDD, dso::HMSFormat::HHMMSSF,
                       dso::nanoseconds>(line + 2);
  } catch (std::exception &e) {
    fprintf(
        stderr,
        "[ERROR] Failed resolvind date for observation block (traceback: %s)\n",
        __func__);
    fprintf(stderr, "[ERROR] Erronuous line was: %s (traceback: %s)\n", line,
            __func__);
    return 2;
  }

  /* probably i am exagerating a bit here, but nevertheless ...
   * it could happen that the 'Epoch flag' and the 'Number of stations' fields
   * are joined in one big int (if number of stations is >=100). Hence, just to
   * be safe, we are moving the fields in a temporary buffer and parse from
   * there.
   */
  char tbuf[4] = {'\0'};

  /* resolve flag */
  std::memcpy(tbuf, line + 31, 3);
  auto cres = std::from_chars(skipws(tbuf), tbuf + 4, val);
  if (cres.ec != std::errc{}) {
    status += 1;
    fprintf(stderr,
            "[ERROR] Failed resolving epoch flag from line: \'%s\' (traceback: "
            "%s)\n",
            line, __func__);
  }
  hdr.m_flag = val;

  /* resolve number of stations */
  std::memcpy(tbuf, line + 34, 3);
  cres = std::from_chars(skipws(tbuf), tbuf + 4, val);
  if (cres.ec != std::errc{}) {
    status += 1;
    fprintf(stderr,
            "[ERROR] Failed resolving #stations from line: \'%s\' (traceback: "
            "%s)\n",
            line, __func__);
  }
  hdr.m_num_stations = val;

  /* if the clock correction field is not empty, parse the float */
  bool has_clock_offset = false;
  for (int i = 0; i < 13; i++) {
    if (*(line + 43 + i) != ' ') {
      has_clock_offset = true;
      break;
    }
  }
  if (has_clock_offset) {
    cres = std::from_chars(skipws(line + 43), line + sz, hdr.m_clock_offset);
    if (cres.ec != std::errc{}) {
      status += 1;
      fprintf(stderr,
              "[ERROR] Failed resolving clock offset from line: \'%s\' "
              "(traceback: %s)\n",
              line, __func__);
    }
  } else {
    hdr.m_clock_offset = dso::doris_rnx::RECEIVER_CLOCK_OFFSET_MISSING;
  }

  /* receiver clock offset flag */
  cres = std::from_chars(skipws(line + 56), line + sz, val);
  if (cres.ec != std::errc{}) {
    status += 1;
    fprintf(stderr,
            "[ERROR] Failed resolving clock flag from line: \'%s\' (traceback: "
            "%s)\n",
            line, __func__);
  }
  hdr.m_clock_flag = val;

  return status;
}

} /* unnamed namespace */

int dso::DorisObsRinex::get_next_data_block(
    dso::doris_rnx::DataBlock &block) noexcept {
  char line[MAX_RECORD_CHARS];

  /* first get and parse the block header (should be next line to be read) */
  if (!m_stream.getline(line, MAX_RECORD_CHARS)) {
    if (m_stream.eof()) {
      /* EOF encountered */
      return -1;
    }
    fprintf(stderr,
            "[ERROR] Failed reading line from stream! (traceback: %s)\n",
            __func__);
    return 1;
  }

  if (resolve_block_epoch(line, block.mheader)) {
    fprintf(stderr,
            "[ERROR] Failed reading data block header! (traceback: %s)\n",
            __func__);
    return 1;
  }

  /* clear observations of block */
  block.mbeacon_obs.clear();

  /* reserve space */
  block.mbeacon_obs.reserve(block.mheader.m_num_stations);

  /* buffers */
  char buf[16] = {'\0'};
  char *end;
  double val;

  /* for every beacon in the block */
  for (int beacon = 0; beacon < block.mheader.m_num_stations; beacon++) {
    /* create emtpy beacon observation array */
    block.mbeacon_obs.emplace_back(dso::doris_rnx::BeaconObservations{});

    /* and get an iterator to it (so that we set its values in-place) */
    auto it = block.mbeacon_obs.end() - 1;

    int curobs = 0;  /* current observation count for beacon */
    int curline = 0; /* current data line of beacon */

    /* for every observation code described in the RINEX header ... */
    while (curobs < (int)m_obs_codes.size()) {
      /* should we change/get the next line ? */
      if (!(curobs % dso::doris_rnx::MAX_OBS_PER_DATA_LINE)) {
        m_stream.getline(line, MAX_RECORD_CHARS);
        /* if this is the first data line for the beacon, get its code */
        if (!curline) {
          if ((*line) != 'D') {
            fprintf(stderr,
                    "[ERROR] Expected line to start with new beacon, found "
                    "something else instead! (traceback: %s)\n",
                    __func__);
            fprintf(stderr, "[ERROR] Erronuous line was: %s (traceback: %s)\n",
                    line, __func__);
            return 1;
          }
          std::memcpy(it->m_beacon_id, line, 3);
        }
        ++curline;
      }

      /* parse observations, one at a time */
      std::memcpy(
          buf, line + 3 + (curobs % dso::doris_rnx::MAX_OBS_PER_DATA_LINE) * 16,
          14);

      /* check if value is ommited (an ommited value is either left blank, or
       * is recorded as 0.0)
       */
      bool buf_is_empty = true;
      end = buf;
      while (*end) {
        if (!isspace(*end++)) {
          buf_is_empty = false;
          break;
        }
      }

      char flagm1 =
          line[3 + (curobs % dso::doris_rnx::MAX_OBS_PER_DATA_LINE) * 16 + 14];
      char flagm2 =
          line[3 + (curobs % dso::doris_rnx::MAX_OBS_PER_DATA_LINE) * 16 + 15];

      if (buf_is_empty) {
        val = dso::doris_rnx::OBSERVATION_VALUE_MISSING;
      } else {
        auto tres = std::from_chars(skipws(buf), buf + 16, val);
        if (!(tres.ec == std::errc{})) {
          fprintf(stderr,
                  "[ERROR] Failed resolving line [%s]/[%s] (traceback: %s)\n",
                  line, buf, __func__);
          return 2;
        }
      }

      /* push value to the current BeaconObservations instance (in-place)
       * WAIT! check if we have a scale factor for the observable (note that
       * the m_obs_scale_factors are in one-to-one correspondance with the
       * m_obs_codes vector. Hence, we can find the scale factor simply by the
       * index of the observable. If no scale factor for the observable exists,
       * then the m_obs_scale_factors should have an '1' in the corresponding
       * index.
       */
      val /= m_obs_scale_factors[curobs];
      it->m_values.emplace_back(val, flagm1, flagm2);
      ++curobs;

    } /* for every observation code described in the RINEX header */
  } /* for every beacon in the block */

  return 0;
} /* end function */
