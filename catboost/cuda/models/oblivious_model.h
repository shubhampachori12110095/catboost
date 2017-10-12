#pragma once

#include <catboost/cuda/data/feature.h>
#include <catboost/cuda/data/binarizations_manager.h>
#include <util/generic/vector.h>

struct TObliviousTreeStructure {
    yvector<TBinarySplit> Splits;

    ui64 GetHash() const {
        return static_cast<ui64>(TVecHash<TBinarySplit>()(Splits));
    }

    ui32 GetDepth() const {
        return static_cast<ui32>(Splits.size());
    }

    ui32 LeavesCount() const {
        return static_cast<ui32>(1 << GetDepth());
    }

    bool HasSplit(const TBinarySplit& candidate) {
        for (const auto& split : Splits) {
            if (split == candidate) {
                return true;
            }
        }
        return false;
    }

    bool operator==(const TObliviousTreeStructure& other) const {
        return Splits == other.Splits;
    }

    bool operator!=(const TObliviousTreeStructure& other) const {
        return !(*this == other);
    }

    SAVELOAD(Splits);
};

template <>
struct THash<TObliviousTreeStructure> {
    inline size_t operator()(const TObliviousTreeStructure& value) const {
        return value.GetHash();
    }
};

class TObliviousTreeModel {
public:
    TObliviousTreeModel(TObliviousTreeStructure&& modelStructure,
                        const yvector<float>& values)
        : ModelStructure(std::move(modelStructure))
        , LeafValues(values)
    {
    }

    TObliviousTreeModel() = default;

    TObliviousTreeModel(const TObliviousTreeStructure& modelStructure)
        : ModelStructure(modelStructure)
        , LeafValues(modelStructure.LeavesCount())
    {
    }

    ~TObliviousTreeModel() {
    }

    const TObliviousTreeStructure& GetStructure() const {
        return ModelStructure;
    }

    inline void Rescale(double scale) {
        for (uint i = 0; i < LeafValues.size(); ++i) {
            LeafValues[i] *= scale;
        }
    }

    void UpdateLeaves(yvector<float>&& newValues) {
        LeafValues = std::move(newValues);
    }

    const yvector<float>& GetValues() const {
        return LeafValues;
    }

    SAVELOAD(ModelStructure, LeafValues);

private:
    TObliviousTreeStructure ModelStructure;
    yvector<float> LeafValues;
};