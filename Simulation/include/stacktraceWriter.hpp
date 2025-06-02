/*
 * CoDiPack, a Code Differentiation Package
 *
 * Copyright (C) 2015-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Homepage: http://www.scicomp.uni-kl.de
 * Contact:  Prof. Nicolas R. Gauger (codi@scicomp.uni-kl.de)
 *
 * Lead developers: Max Sagebaum, Johannes Blühdorn (SciComp, TU Kaiserslautern)
 *
 * This file is part of CoDiPack (http://www.scicomp.uni-kl.de/software/codi).
 *
 * CoDiPack is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * CoDiPack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU
 * General Public License along with CoDiPack.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * For other licensing options please contact us.
 *
 * Authors:
 *  - SciComp, TU Kaiserslautern:
 *    - Max Sagebaum
 *    - Johannes Blühdorn
 *    - Former members:
 *      - Tim Albring
 */
#pragma once

#include <execinfo.h>
#include <vector>
#include <link.h>

#include "stringUtil.hpp"

/** \copydoc codi::Namespace */
namespace codi {

  struct StacktraceWriter {
    public:

      int maxSize;
      int skip;
      bool dladdrtrans;

      std::vector<void*> trace;

      StacktraceWriter(int maxSize = 15, int skip = 2, bool dladdrtrans=false) : maxSize(maxSize), skip(skip), dladdrtrans(dladdrtrans), trace(maxSize) {}


      template<typename Stream>
      void writeStacktrace(Stream& stream) {

        Dl_info info;
        link_map* link_map;
        int traceSize = backtrace(trace.data(), maxSize);

        if(dladdrtrans) {
          for (int i = skip; i < traceSize; ++i) {
            //dladdr1((void*)trace[i], &info, (void**)&link_map, RTLD_DL_LINKMAP);
            stream << StringUtil::format("%#x ", (char*)trace[i] - link_map->l_addr);
          }
        } else {
          for (int i = skip; i < traceSize; ++i) {
            stream << StringUtil::format("%#x ", trace[i]);
          }
        }
        stream << "\n";
      }

      template<typename Stream>
      void writeStacktraceBinary(Stream& stream, int lhsIdentifier) {

         int trace_size = backtrace(trace.data(), maxSize);

        int size = trace_size - skip;
        stream.write((char const*)(&lhsIdentifier), sizeof(int));
        stream.write((char const*)(&size), sizeof(int));
        stream.write((char const*)(&trace[skip]), sizeof(void*) * size);
      }
  };
}
