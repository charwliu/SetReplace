/**
 * MIT License
 *
 * Copyright (c) 2018 Maksim Piskunov

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef Match_hpp
#define Match_hpp

#include <vector>
#include <set>

#include "id_types.h"
#include "expression.h"
#include "rule.h"

namespace SetReplace {
    /** @brief Match is a potential event that has not actualized yet.
     */
    struct Match {
        /** @brief ID for the rule this match corresponds to.
         */
        RuleID rule;

        /** @brief Expression matching the rule inputs.
         */
        std::vector<ExpressionID> inputExpressions;
    };

    using MatchPtr = std::shared_ptr<const Match>;

    /** @brief Matcher takes rules, atoms index, and a list of expressions, and returns all possible matches.
     * @details This contains the lowest-level code, and the main functionality of the library. Uses atomsIndex to discover expressions, thus if an expression is absent from the atomsIndex, it would not appear in any matches.
     */
    class Matcher {
     public:
        /** @brief Type of the error occurred during evaluation.
         */
        enum Error { Aborted, DisconnectedInputs, NoMatches, InvalidOrderingFunction, InvalidOrderingDirection };

        /** @brief All possible functions available to sort matches. Random is the default that is always applied last.
         */
        enum class OrderingFunction {
            SortedExpressionIDs = 0,
            ReverseSortedExpressionIDs = 1,
            ExpressionIDs = 2,
            RuleID = 3
        };

        /** @brief Whether to sort in normal or reverse order.
         */
        enum class OrderingDirection { Normal = 0, Reverse = 1 };

        /** @brief Full specification for the sequence of ordering functions.
         */
        using OrderingSpec = std::vector<std::pair<OrderingFunction, OrderingDirection>>;

        /** @brief Creates a new matcher object.
         * @details This is an O(1) operation, does not do any matching yet.
         */
        Matcher(const std::vector<Rule>& rules,
                AtomsIndex& atomsIndex,
                const std::function<AtomsVector(ExpressionID)> getAtomsVector,
                const OrderingSpec orderingSpec,
                const unsigned int randomSeed = 0);

        /** @brief Finds and adds to the index all matches involving specified expressions.
         * @details Calls shouldAbort() frequently, and throws Error::Aborted if that returns true. Otherwise might take significant time to evaluate depending on the system.
         */
        void addMatchesInvolvingExpressions(const std::vector<ExpressionID>& expressionIDs,
                                            const std::function<bool()> shouldAbort);

        /** @brief Removes matches containing specified expression IDs from the index.
         */
        void removeMatchesInvolvingExpressions(const std::vector<ExpressionID>& expressionIDs);

        /** @brief Yields true if there are no matches left.
         */
        bool empty() const;

        /** @brief Returns the match that should be substituted next.
         * @details Throws Error::NoMatches if there are no matches.
         */
        MatchPtr nextMatch() const;

        /** @brief Replaces patterns in atomsToReplace with explicit atoms.
         * @param inputPatterns patterns corresponding to patternMatches.
         * @param patternMatches explicit atoms corresponding to patterns in inputPatterns.
         * @param atomsToReplace patterns, which would be replaced the same way as inputPatterns are matched to patternMatches.
         */
        static bool substituteMissingAtomsIfPossible(const std::vector<AtomsVector> inputPatterns,
                                                     const std::vector<AtomsVector> patternMatches,
                                                     std::vector<AtomsVector>& atomsToReplace);

        /** @brief Returns the set of expression IDs matched in any match. */
        const std::vector<MatchPtr> allMatches() const;

     private:
        class Implementation;
        std::shared_ptr<Implementation> implementation_;
    };
}

#endif /* Match_hpp */