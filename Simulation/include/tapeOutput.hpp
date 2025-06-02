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

#include <iostream>
#include <fstream>

#include <codi/expressions/lhsExpressionInterface.hpp>
#include <codi/misc/eventSystem.hpp>
#include <codi/misc/macros.hpp>
#include <codi/tapes/data/emptyData.hpp>
#include <codi/tapes/interfaces/fullTapeInterface.hpp>

#include "stacktraceWriter.hpp"

/** \copydoc codi::Namespace */
namespace codi {

  template<typename T_CoDiType>
  struct TapeOutput {
    private:

      using CoDiType = CODI_DD(T_CoDiType, CODI_DEFAULT_LHS_EXPRESSION);
      using Tape = typename CoDiType::Tape;
      using Real = typename CoDiType::Real;
      using Identifier = typename CoDiType::Identifier;

      using Handle = typename EventSystem<Tape>::Handle;

      bool statementRecordingActive;
      std::ofstream statementPrimal;
      std::ofstream statementAdjoint;
      std::ofstream statementIndex;
      std::ofstream statementStacktrace;

      Handle statementStoreOnTapeHandle;

      bool evaluationRecordingActive;
      std::ofstream evaluationPrimal;
      std::ofstream evaluationAdjoint;
      std::ofstream evaluationIndex;
      std::ofstream evaluationPrimalIndex;

      Handle evaluateStatementEvaluateHandle;
      Handle evaluateStatementPrimalEvaluateHandle;

      bool binaryStackTrace;

      StacktraceWriter stacktraceWriter;

    public:

      TapeOutput()
          : statementRecordingActive(false),
            statementPrimal(),
            statementAdjoint(),
            statementIndex(),
            statementStacktrace(),
            statementStoreOnTapeHandle(),
            evaluationRecordingActive(false),
            evaluationPrimal(),
            evaluationAdjoint(),
            evaluationIndex(),
            evaluationPrimalIndex(),
            evaluateStatementEvaluateHandle(),
            evaluateStatementPrimalEvaluateHandle(),
            binaryStackTrace(false),
            stacktraceWriter(15, 2)
      {}

      void startStatementRecording(const std::string& namePrefix, bool primal, bool jacobian, bool index, bool stacktrace) {
        statementRecordingActive = true;
        if(primal) {
          openFloatingpointStream(statementPrimal, namePrefix + "_stmt_primal.out");
        }

        if(jacobian) {
          openFloatingpointStream(statementAdjoint, namePrefix + "_stmt_jacobian.out");
        }

        if(index) {
          openIndexStream(statementIndex, namePrefix + "_stmt_index.out");
        }

        if(stacktrace) {
          if(binaryStackTrace) {
            statementStacktrace.open(namePrefix + "_stmt_stack.out", std::ios::binary);
          } else {
            statementStacktrace.open(namePrefix + "_stmt_stack.out");
          }
        }

        statementStoreOnTapeHandle = EventSystem<Tape>::registerStatementStoreOnTapeListener(handleStatementRecord, this);
      }

      void stopStatementRecording() {

        EventSystemBase<Tape>::deregisterListener(statementStoreOnTapeHandle);

        statementRecordingActive = false;
        if(statementPrimal) { statementPrimal.close();}
        if(statementAdjoint) {statementAdjoint.close();}
        if(statementStacktrace) {statementStacktrace.close();}
      }

      void startEvaluationReverseRecording(const std::string& namePrefix, bool primal, bool adjoint, bool index) {
        if(evaluationRecordingActive) {
          CODI_EXCEPTION("Recording of evaluation already active.");
        }

        evaluationRecordingActive = true;

        if(primal) {
          openFloatingpointStream(evaluationPrimal, namePrefix + "_eval_reverse_primal.out");
        }

        if(adjoint) {
          openFloatingpointStream(evaluationAdjoint, namePrefix + "_eval_reverse_adjoint.out");
        }

        if(index) {
          openIndexStream(evaluationIndex, namePrefix + "_eval_reverse_index.out");
        }

        registerEvaluateEvents();
      }


      void startEvaluationForwardRecording(const std::string& namePrefix, bool primal, bool tangent, bool index) {
        if(evaluationRecordingActive) {
          CODI_EXCEPTION("Recording of evaluation already active.");
        }

        evaluationRecordingActive = true;

        if(primal) {
          openFloatingpointStream(evaluationPrimal, namePrefix + "_eval_forward_primal.out");
        }

        if(tangent) {
          openFloatingpointStream(evaluationAdjoint, namePrefix + "_eval_forward_adjoint.out");
        }

        if(index) {
          openIndexStream(evaluationIndex, namePrefix + "_eval_forward_index.out");
        }

        registerEvaluateEvents();
      }

      void startEvaluationPrimaldRecording(const std::string& namePrefix, bool primal, bool index) {
        if(evaluationRecordingActive) {
          CODI_EXCEPTION("Recording of evaluation already active.");
        }

        evaluationRecordingActive = true;

        if(primal) {
          openFloatingpointStream(evaluationPrimal, namePrefix + "_eval_primal_primal.out");
        }

        if(index) {
          openIndexStream(evaluationPrimalIndex, namePrefix + "_eval_primal_index.out");
        }

        registerEvaluateEvents();
      }

      void stopEvaluationRecording() {
        deregisterEvaluateEvents();

        evaluationRecordingActive = false;
        if(evaluationPrimal) { evaluationPrimal.close(); }
        if(evaluationAdjoint) { evaluationAdjoint.close(); }
        if(evaluationIndex) { evaluationIndex.close(); }
        if(evaluationPrimalIndex) { evaluationPrimalIndex.close(); }
      }

    private:

      void openFloatingpointStream(std::ofstream& stream, std::string const& file) {
        stream.open(file);
        stream.setf(std::ios::scientific);
        stream.setf(std::ios::showpos);
        stream.precision(15);
      }

      void openIndexStream(std::ofstream& stream, std::string const& file) {
        stream.open(file);
        stream.setf(std::ios::scientific);
        stream.setf(std::ios::showpos);
        stream.precision(15);
      }

      void registerEvaluateEvents() {
        evaluateStatementEvaluateHandle = EventSystem<Tape>::registerStatementEvaluateListener(handleStatementEvaluate, this);
        evaluateStatementPrimalEvaluateHandle = EventSystem<Tape>::registerStatementEvaluatePrimalListener(handleStatementPrimalEvaluate, this);

      }

      void deregisterEvaluateEvents() {
        EventSystemBase<Tape>::deregisterListener(evaluateStatementEvaluateHandle);
        EventSystemBase<Tape>::deregisterListener(evaluateStatementPrimalEvaluateHandle);
      }

      static void handleStatementRecord(Tape& tape, Identifier const& lhsIdentifier, Real const& newValue,
                                 size_t numActiveVariables, Identifier const* rhsIdentifiers, Real const* jacobians,
                                 void* userData) {

        TapeOutput* output = static_cast<TapeOutput*>(userData);

        if(output->statementRecordingActive) {
          if(output->statementPrimal) {
            output->statementPrimal << newValue << "\n";
          }

          if(output->statementAdjoint) {
            output->statementAdjoint << numActiveVariables;

            // write jacobie values
            for(size_t i = 0; i < numActiveVariables; ++i) {
              output->statementAdjoint << " " << jacobians[i];
            }

            // second loop prints the indices:
            output->statementAdjoint << " " << lhsIdentifier;
            for(size_t i = 0; i < numActiveVariables; ++i) {
              output->statementAdjoint << " " << rhsIdentifiers[i];
            }
            output->statementAdjoint << "\n";
          }

          if(output->statementIndex) {
            output->statementIndex << lhsIdentifier << "\n";
          }

          if(output->statementStacktrace) {
            if(output->binaryStackTrace) {
              output->stacktraceWriter.writeStacktraceBinary(output->statementStacktrace, (int)lhsIdentifier);
            } else {
              output->stacktraceWriter.writeStacktrace(output->statementStacktrace);
            }
          }
        }
      }

      static void handleStatementEvaluate(Tape& tape, Identifier const& lhsIdentifier, size_t sizeLhsAdjoint,
                                          Real const* lhsAdjoint, void* userData) {
        TapeOutput* output = static_cast<TapeOutput*>(userData);

        if(output->evaluationRecordingActive) {
          if(output->evaluationAdjoint) {
            for(size_t i = 0; i < sizeLhsAdjoint; ++i) {
              if(i != 0) {
                output->evaluationAdjoint << " ";
              }
              output->evaluationAdjoint <<  lhsAdjoint[i];
            }
            output->evaluationAdjoint << "\n";
          }

          if(output->evaluationIndex) {
            output->evaluationIndex << lhsIdentifier << "\n";
          }
        }
      }

      static void handleStatementPrimalEvaluate(Tape& tape, Identifier const& lhsIdentifier,
                                                Real const& lhsValue, void* userData) {
        TapeOutput* output = static_cast<TapeOutput*>(userData);

        if(output->evaluationRecordingActive) {
          if(output->evaluationPrimal) {
            output->evaluationPrimal <<  lhsValue << "\n";
          }

          if(output->evaluationPrimalIndex) {
            output->evaluationPrimalIndex << lhsIdentifier << "\n";
          }
        }
      }
  };
}
