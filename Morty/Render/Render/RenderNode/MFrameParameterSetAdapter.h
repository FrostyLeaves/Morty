/**
 * @File         MFrameParameterSetAdapter
 *
 * @Created      2025-01-12
 *
 * @Author       DoubleYe
 * @Brief        Adapter class for passing ParameterSet through render graph.
 *               Inherits only from MTypeClass (no multiple inheritance).
 **/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Type/MType.h"

#include <memory>

namespace morty
{

class MShaderParameterSet;

/**
 * Adapter class for passing ParameterSet through render graph.
 * Provides the same interface as IParameterSetAdapter but inherits from MTypeClass
 * to work with the render graph data passing system.
 */
class MORTY_API MFrameParameterSetAdapter : public MTypeClass
{
    MORTY_CLASS(MFrameParameterSetAdapter)

public:
    MFrameParameterSetAdapter() = default;

    explicit MFrameParameterSetAdapter(std::shared_ptr<MShaderParameterSet> paramSet)
        : m_parameterSet(std::move(paramSet))
    {}

    [[nodiscard]] std::shared_ptr<MShaderParameterSet> GetParameterSet() const { return m_parameterSet; }

    void SetParameterSet(std::shared_ptr<MShaderParameterSet> paramSet) { m_parameterSet = std::move(paramSet); }

private:
    std::shared_ptr<MShaderParameterSet> m_parameterSet;
};

}// namespace morty
