/*  This file is part of YUView - The YUV player with advanced analytics toolset
 *   <https://github.com/IENT/YUView>
 *   Copyright (C) 2015  Institut f�r Nachrichtentechnik, RWTH Aachen University, GERMANY
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the
 *   OpenSSL library under certain conditions as described in each
 *   individual source file, and distribute linked combinations including
 *   the two.
 *
 *   You must obey the GNU General Public License in all respects for all
 *   of the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the
 *   file(s), but you are not obligated to do so. If you do not wish to do
 *   so, delete this exception statement from your version. If you delete
 *   this exception statement from all source files in the program, then
 *   also delete it here.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "sps_range_extension.h"

namespace parser::hevc
{

using namespace reader;

void sps_range_extension::parse(SubByteReaderLogging &reader)
{
  SubByteReaderLoggingSubLevel subLevel(reader, "sps_range_extension()");

  this->transform_skip_rotation_enabled_flag =
      reader.readFlag("transform_skip_rotation_enabled_flag");
  this->transform_skip_context_enabled_flag =
      reader.readFlag("transform_skip_context_enabled_flag");
  this->implicit_rdpcm_enabled_flag        = reader.readFlag("implicit_rdpcm_enabled_flag");
  this->explicit_rdpcm_enabled_flag        = reader.readFlag("explicit_rdpcm_enabled_flag");
  this->extended_precision_processing_flag = reader.readFlag("extended_precision_processing_flag");
  this->intra_smoothing_disabled_flag      = reader.readFlag("intra_smoothing_disabled_flag");
  this->high_precision_offsets_enabled_flag =
      reader.readFlag("high_precision_offsets_enabled_flag");
  this->persistent_rice_adaptation_enabled_flag =
      reader.readFlag("persistent_rice_adaptation_enabled_flag");
  this->cabac_bypass_alignment_enabled_flag =
      reader.readFlag("cabac_bypass_alignment_enabled_flag");
}

} // namespace parser::hevc
