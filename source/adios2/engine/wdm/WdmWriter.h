/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * WdmWriter.h
 *
 *  Created on: Nov 1, 2018
 *      Author: Jason Wang
 */

#ifndef ADIOS2_ENGINE_STAGINGWRITER_H_
#define ADIOS2_ENGINE_STAGINGWRITER_H_

#include <queue>

#include "adios2/core/Engine.h"
#include "adios2/toolkit/format/dataman/DataManSerializer.h"
#include "adios2/toolkit/format/dataman/DataManSerializer.tcc"
#include "adios2/toolkit/transportman/stagingman/StagingMan.h"

namespace adios2
{
namespace core
{
namespace engine
{

class WdmWriter : public Engine
{

public:
    WdmWriter(IO &adios, const std::string &name, const Mode mode,
              MPI_Comm mpiComm);

    ~WdmWriter() = default;

    StepStatus BeginStep(
        StepMode mode,
        const float timeoutSeconds = std::numeric_limits<float>::max()) final;
    size_t CurrentStep() const final;
    void PerformPuts() final;
    void EndStep() final;
    void Flush(const int transportIndex = -1) final;

private:
    int m_Channels = 1;
    int64_t m_QueueLimit = 3000;
    std::string m_QueueFullPolicy = "discard";
    size_t m_DefaultBufferSize = 1024;
    int m_Port = 12307;

    format::DataManSerializer m_DataManSerializer;
    int64_t m_CurrentStep = -1;
    int m_MpiRank;
    int m_MpiSize;
    std::vector<std::string> m_FullAddresses;
    int m_Timeout = 5;
    bool m_Listening = false;
    bool m_Tolerance = true;
    bool m_AttributesSet = false;
    int m_AppID = 0;

    void Init() final;
    void InitParameters() final;
    void InitTransports() final;
    void Handshake();

    void ReplyThread(const std::string &address);
    std::vector<std::thread> m_ReplyThreads;

#define declare_type(T)                                                        \
    void DoPutSync(Variable<T> &, const T *) final;                            \
    void DoPutDeferred(Variable<T> &, const T *) final;
    ADIOS2_FOREACH_STDTYPE_1ARG(declare_type)
#undef declare_type

    /**
     * Closes a single transport or all transports
     * @param transportIndex, if -1 (default) closes all transports,
     * otherwise it closes a transport in m_Transport[transportIndex].
     * In debug mode the latter is bounds-checked.
     */
    void DoClose(const int transportIndex = -1) final;

    /**
     * Common function for primitive PutSync, puts variables in buffer
     * @param variable
     * @param values
     */
    template <class T>
    void PutSyncCommon(Variable<T> &variable, const T *values);

    template <class T>
    void PutDeferredCommon(Variable<T> &variable, const T *values);

    void Log(const int level, const std::string &message, const bool mpi,
             const bool endline);

    int m_Verbosity = 0;
};

} // end namespace engine
} // end namespace core
} // end namespace adios2

#endif // ADIOS2_ENGINE_STAGINGWRITER_H_
