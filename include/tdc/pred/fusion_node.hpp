#pragma once

#include <cstddef>
#include <cstdint>

#include <tdc/pred/fusion_node_internals.hpp>
#include <tdc/util/concepts.hpp>
#include <tdc/util/skip_accessor.hpp>

#include "result.hpp"

namespace tdc {
namespace pred {

/// \brief A compressed trie that can solve predecessor queries for up to 8 64-bit keys using only 128 bits.
/// \tparam the key type
/// \tparam the maximum number of keys
template<std::totally_ordered key_t = uint64_t, size_t m_max_keys = 8>
class FusionNode {
private:
    using Internals = internal::FusionNodeInternals<key_t, m_max_keys, false>;

    using mask_t = typename Internals::mask_t;
    using matrix_t = typename Internals::matrix_t;

    key_t    m_key_[m_max_keys];
    mask_t   m_mask_;
    matrix_t m_branch_, m_free_;

public:
    /// \brief Constructs an empty compressed trie.
    FusionNode(): m_mask_(0) {
    }

    /// \brief Constructs a compressed trie for the given keys.
    ///
    /// Note that the keys are \em not stored in the trie.
    /// This is to keep this data structure as small as possible.
    /// The keys will, howver, be needed for predecessor lookups and thus need to be managed elsewhere.
    ///
    /// \param keys a pointer to the keys, that must be in ascending order
    /// \param num the number of keys, must be at most \ref MAX_KEYS
    template<IndexAccessTo<key_t> keyarray_t>
    FusionNode(const keyarray_t& keys, const size_t num) {
        auto fnode8 = Internals::template construct<key_t>(keys, num);
        m_mask_ = std::get<0>(fnode8);
        m_branch_ = std::get<1>(fnode8);
        m_free_ = std::get<2>(fnode8);
        std::copy(keys, keys + num, m_key_);
    }

    /// \brief Finds the rank of the predecessor of the specified key in the compressed trie.
    /// \param keys the keys that the compressed trie was constructed for
    /// \param x the key in question
    template<IndexAccessTo<key_t> keyarray_t>
    PosResult predecessor(const keyarray_t& keys, const key_t x) const {
        return Internals::predecessor(keys, x, m_mask_, m_branch_, m_free_);
    }

    PosResult predecessor(const key_t x) const {
        return Internals::predecessor(m_key_, x, m_mask_, m_branch_, m_free_);
    }

    const key_t * m_key() const { return m_key_; }

    const mask_t m_mask() const { return m_mask_; }

    const matrix_t m_branch() const { return m_branch_; }

    const matrix_t m_free() const { return m_free_; }
};

}} // namespace tdc::pred
